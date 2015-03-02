#!/usr/bin/python
import sys
sys.path.append('../python')

import logging
from rmqwrap.server import RmqHandler
from rmqwrap import common
import argparse

LOG = logging.getLogger('report-server')

class RmqReport(RmqHandler):
    def __init__(self, host):
        RmqHandler.__init__(self, host, user_handler=None)
        self._user_handler = user_handler

        if not self.conn_.queue(queue_name=common.gen_queue_name('REPORT'),
                                durable=True):
            raise Exception('Cannot create queue')

    def run(self):
        if not self.conn_.consumeBasic(self.handler, queue=self.conn_.queueName(), no_ack=True):
            raise Exception('Cannot register consumer')
        super(RmqReport, self).run()

    def handler(self, ch, method, props, body):
        LOG.debug('REPORT: ', ch, method, props, body)
        if self._user_handler:
            self._user_handler(body)


def handle(user_handler=None):
    # create objects
    rReport = RmqReport('172.27.0.11', user_handler)

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
    parser = argparse.ArgumentParser(description='Test REPORT sever')
    parser = common.add_default_options(parser)
    parser.add_argument('script', help='Client script')
    parser.add_argument('args', nargs=argparse.REMAINDER,
                        help='Client script arguments')
    options = parser.parse_args()
    common.process_default_options(options)

    logging.basicConfig(level=logging.DEBUG)

    action_file = options.script
    logging.debug('import actions file: %s', action_file)
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

    if not handle(lambda data: handler(data, args)):
        sys.exit("error, bye")

    LOG.debug("...end.")


if __name__ == "__main__":
    main()
