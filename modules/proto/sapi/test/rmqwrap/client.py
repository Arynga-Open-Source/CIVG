'''RMQ client'''
import uuid
import common
import logging
try:
    import pika
except ImportError:
    print 'python-pika is missing'

LOG = logging.getLogger('rmq.client')

class CarSyncReq(object):
    def __init__(self, host):
        LOG.info('connect to %s', host)
        self.connection = common.new_connection(host)
        LOG.info('connection to %s opened', host)

        self.channel = self.connection.channel()
        LOG.info('got channel')

        result = common.get_anon_queue(self.channel,
                                       exclusive=True,
                                       auto_delete=True)
        LOG.info('got anonymous queue: %s', result.method.queue)

        self.callback_queue = result.method.queue

        LOG.info('setting up consumer')
        tag = self.channel.basic_consume(self.on_response, no_ack=False,
                                         queue=self.callback_queue)
        LOG.info('got cosumer tag: %s', tag)

        # default correlation id
        self.corr_id = None

    def on_response(self, ch, method, props, body):
        """callback on receiving a message in queue"""
        assert(self.corr_id != None)

        LOG.debug('response in queue, self corr ID: %s, other ID: %s',
                  self.corr_id, props.correlation_id)
        if self.corr_id == props.correlation_id:
            self.response = body

    def call(self, queue_name, body=None, wait_for_response=True):
        """issue an RPC call, sending `body` to queue named
        `queue_name`
        """
        if body == None or queue_name == None:
            return None

        self.response = None
        self.corr_id = str(uuid.uuid4())

        LOG.debug('send message to queue %s', queue_name)
        LOG.debug('  expect response in queue %s', self.callback_queue)
        LOG.debug('  correlation ID: %s', self.corr_id)
        props = pika.BasicProperties(reply_to = self.callback_queue,
                                     correlation_id = self.corr_id)
        common.send_to_queue(self.channel, queue_name, body,
                             properties=props)

        if wait_for_response:
            try:
                LOG.info('wait for response')
                while self.response is None:
                    self.connection.process_data_events()
            except KeyboardInterrupt, key:
                LOG.info('keyboard interrupt')
                self.response = None
            else:
                LOG.info('  got response')
        else:
            LOG.info('not waiting for response')
        self.corr_id = None
        return self.response

    def release(self, ):
        """release channel and connections
        """
        # releases all channels inside
        self.connection.close()


class Sink(object):
    """Drain qiven queue
    """

    def __init__(self, host, qname):
        """
        """
        # reset receive counter
        self._got = 0
        # queue name
        self._qname = qname
        # timer id
        self._timer_id = None
        # done flag
        self._run = False
        # default wait timeout
        self._timeout = 5

        LOG.info('connect to %s', host)
        self.connection = common.new_connection(host)
        LOG.info('connection to %s opened', host)

        self.channel = self.connection.channel()
        LOG.info('got channel')



    def on_recv(self, ch, method, props, body):
        """sink

        Arguments:
        - `ch`:
        - `method`:
        - `props`:
        - `body`:
        """
        #LOG.debug('drain')
        self._got += 1
        if self._got % 10 == 0:
            LOG.info(' ... got %d', self._got)
        if self._timer_id:
            self.connection.remove_timeout(self._timer_id)

        self._timer_id = self.connection.add_timeout(self._timeout,
                                                     self.on_timeout)

    def on_timeout(self):
        """
        """
        LOG.info('data wait timeout')
        self._run = False

    def consume(self, ):
        """
        """
        LOG.info('starting to consume messages')

        self._run = True

        self._timer_id = self.connection.add_timeout(self._timeout,
                                                     self.on_timeout)

        self.channel.basic_consume(self.on_recv, no_ack=True,
                                   queue=self._qname)

        while self._run:
            try:
                self.connection.process_data_events();
            except KeyboardInterrupt:
                LOG.info('breaking on user\'s request')
                break

        LOG.info('got %d messages', self._got)

    def release(self, ):
        """
        """
        self.connection.close()

