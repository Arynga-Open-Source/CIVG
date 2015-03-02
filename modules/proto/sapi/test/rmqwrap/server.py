#!/usr/bin/env python
import pika
import logging
import threading

LOG = logging.getLogger('rmq.server')

class rmqConn:
    def __init__(self, addr, port=5672):
        LOG.debug('connect to %s port, %d', addr, port)
        self.addr_ = addr
        self.connParams_ = pika.ConnectionParameters(host=addr, port=port)
        self.conn_ = None
        self.ch_ = None
        self.q_ = ''
        self.q_durable_ = False


    def connect(self):
        return False

    def disconnect(self):
        if self.ch_:
            if self.q_:
                if not self.q_durable_:
                    self.ch_.queue_delete(queue=self.q_)
                else:
                    LOG.debug('durable queue, not removing')
            self.ch_.close()
        if self.conn_:
            self.conn_.close()
        self.ch_ = None
        self.conn_ = None
        self.q_ = ''

    def queue(self, queue_name='', passive=False, durable=False, exclusive=False, auto_delete=False, nowait=False, arguments=None):
        res = True;

        if not self.ch_ or (self.q_ and self.q_ != queue_name):
            res = False
        else:
            try:
                r = self.ch_.queue_declare( queue=queue_name, passive=passive, durable=durable, exclusive=exclusive, auto_delete=auto_delete, nowait=nowait, arguments=arguments)
                self.q_ = r.method.queue
                self.q_durable_ = durable
            except:
                res = False

        return res

    def queueTmp(self, nowait=False, arguments=None):
        return self.queue(durable=False, auto_delete=True, nowait=nowait, arguments=arguments)

    def queueName(self):
        return self.q_

    def consumeBasic(self, cb, queue='', no_ack=False, exclusive=False, consumer_tag=None):
        LOG.debug('register consumer for queue: %s', queue)
        r = False
        if self.ch_:
            r = True
            try:
                self.ch_.basic_consume(cb, queue=queue, no_ack=no_ack, exclusive=exclusive, consumer_tag=consumer_tag)
                self.exchg_ = queue
            except Exception, e:
                LOG.debug(e)
                raise
                r = False

        return r

    def consumeStart(self):
        return False

    def consumeCancel(self):
        return False

    def exchg(self, exchange=None, exchange_type='direct', passive=False, durable=False, auto_delete=False, internal=False, nowait=False, arguments=None, type=None):
        return False

    def exchgBroadcast(self, exchange=None, arguments=None):
        return False

    def exchgPublish(self, msg):
        return False

    def publish(self):
        return False

    def ack(self, tag):
        r = False
        if self.ch_:
            r = True
            try:
                self.ch_.basic_ack(delivery_tag = tag)
            except:
                r = False
        return r

class rmqConnBlock(rmqConn):
    def __init__(self, addr, port=5672):
        rmqConn.__init__(self, addr, port)

    def connect(self):
        res = True
        try:
            if not self.conn_:
                self.conn_ = pika.BlockingConnection(self.connParams_)
                self.ch_ = self.conn_.channel()
        except:
            res = False
        return res

    def consumeStart(self):
        self.ch_.start_consuming()
        #self.ch_.consume(self.queueName())
        return True

    def consumeCancel(self):
        #self.ch_.basic_cancel()
        self.ch_.stop_consuming()
        return True

    def exchg(self, exchange=None, exchange_type='direct', passive=False, durable=False, auto_delete=False, internal=False, nowait=False, arguments=None, type=None):
        LOG.debug('declare exchange: %s', exchange)
        r = False

        if self.ch_:
            r = True
            try:
                self.ch_.exchange_declare(exchange=exchange, exchange_type=exchange_type, passive=passive, durable=durable, auto_delete=auto_delete, internal=internal, nowait=nowait, arguments=arguments, type=type)
                self.exchg_ = exchange
            except Exception, e:
                r = False
                raise
        else:
            LOG.error('channel not opened?')

        return r

    def exchgBroadcast(self, exchange=None, arguments=None):
        return self.exchg(exchange=exchange, arguments=arguments, type='fanout')

    def exchgPublish(self, msg):
        return self.publish(exchange=self.exchg_, routing_key='', body=msg)

    def publish(self, exchange, routing_key, body,
                      properties=None, mandatory=False, immediate=False):
        r = False

        if self.ch_:
            r = True
            try:
                self.ch_.basic_publish(exchange=exchange, routing_key=routing_key, body=body, properties=properties, mandatory=mandatory, immediate=immediate)
            except Exception, e:
                LOG.debug(e)
                r = False
                raise
        return r


class RmqHandler(object):
    """Generic wrapper for object receving something from RMQ
    """

    def __init__(self, host):
        """
        """
        self.conn_ = rmqConnBlock(host)
        if not self.conn_.connect():
            raise Exception("Cannot create connection")

    def release(self, ):
        """Cleanup
        """
        self.conn_.disconnect()

    def run(self):
        LOG.debug('start consuming...')
        return self.conn_.consumeStart()

    def stop(self):
        LOG.debug('stop consuming...')
        return self.conn_.consumeCancel()


class thRmqHandler(threading.Thread, RmqHandler):
    """Threaded RMQ handler
    """
    def __init__(self, host):
        """
        """
        LOG.debug('init threaded handler')
        threading.Thread.__init__(self)
        RmqHandler.__init__(host)

    def run(self):
        LOG.debug('(thread) start consuming...')
        return RmqHandler.run(self)

    def stop(self):
        LOG.debug('(thread) stop consuming...')
        return RmqHandler.stop(self)
