#!/usr/bin/python
import sys
sys.path.append('../python')

from southapi_pb2 import BaseMessage, RPC

import logging
import pika
import time
from rmqwrap.server import RmqHandler
from rmqwrap import common
import argparse

LOG = logging.getLogger('rpc-server')

class RmqRPC(RmqHandler):
    def __init__(self, host, user_handler=None, simulate_errors=0):
        RmqHandler.__init__(self, host)

        self._user_handler = user_handler

        if not self.conn_.queue(queue_name=common.gen_queue_name('RPC'),
                                durable=True):
            raise Exception('Cannot create queue')
        if not self.conn_.consumeBasic(self.handler, queue=self.conn_.queueName(), no_ack=False):
            raise Exception('Cannot register consumer')

    def run(self):
        LOG.debug('run..')
        super(RmqRPC, self).run()

    def handler(self, ch, method, props, body):
        LOG.debug('RPC request')

        if not self._user_handler:
            return
        else:
            LOG.debug('user handler present')
        response = self._user_handler(body)

        key = props.reply_to
        corr_id = props.correlation_id

        self.conn_.publish(exchange='', routing_key=key,
                           body=response,
                           properties=pika.BasicProperties(correlation_id = corr_id))
        self.conn_.ack(method.delivery_tag)

        # print " [.] Sent response: %s" % (resp,)


def handle(error_simulate=0, user_handler=None):
    # create objects
    rReport = RmqRPC('172.27.0.11',
                     user_handler,
                     error_simulate)

    LOG.debug("RabbitMQ running ...")

    # run all handlers
    try:
        rReport.run()
    except KeyboardInterrupt:
        LOG.debug('interrupted by user')

    LOG.debug("Closing all...")

    # close
    rReport.stop()
    rReport.release()
    return True


def main():
    parser = argparse.ArgumentParser(description='Test RPC server')
    parser = common.add_default_options(parser)
    parser.add_argument('-e', '--simulate-errors', dest='error_sim',
                        help='Simulate error every <n> requests',
                        default=0, type=int)
    parser.add_argument('script', help='Client script')
    parser.add_argument('args', nargs=argparse.REMAINDER,
                        help='Client script arguments')
    options = parser.parse_args()
    common.process_default_options(options)
    action_file = options.script

    logging.basicConfig(level=logging.DEBUG)

    import imp
    actions = imp.load_source('actions', action_file)
    try:
        assert hasattr(actions, 'handle_request')
        handler = actions.handle_request
    except AssertionError:
        print 'required functions: handle_request'
        print '  not found in module %s' % (action_file)
        raise SystemExit(1)

    logging.getLogger('pika').setLevel(logging.INFO)

    LOG.debug("Start RabbitMQ CarSync server ...")

    LOG.debug("----------------------------------")

    if not handle(options.error_sim,
                  lambda data: handler(data, options.args)):
        sys.exit("error, bye")

    LOG.debug("...end.")


if __name__ == "__main__":
    main()
