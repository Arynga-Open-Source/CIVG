#!/usr/bin/python
import sys
sys.path.append('../python')

import logging
from rmqwrap.server import RmqHandler
from rmqwrap import common
import argparse

LOG = logging.getLogger('notify-server')

class RmqNotify(RmqHandler):
    def __init__(self, host):
        RmqHandler.__init__(self, host)

        if not self.conn_.exchgBroadcast(common.gen_queue_name('NOTIFICATION')):
            raise Exception('Cannot create exchg %s' % \
                                (common.gen_queue_name('NOTIFICATION')))


    def run(self, gen_notification):
        """
        """
        self.conn_.exchgPublish(gen_notification())


def handle(gen_notification):
    # create objects
    rReport = RmqNotify('172.27.0.11')

    LOG.debug("RabbitMQ running ...")

    # run all handlers
    try:
        rReport.run(gen_notification)
    except KeyboardInterrupt:
        LOG.debug('interrupted by user')

    LOG.debug("Closing all...")

    # close
    rReport.stop()
    rReport.release()
    return True


def main():
    parser = argparse.ArgumentParser(description='Test NOTIFICATION server')
    parser = common.add_default_options(parser)
    parser.add_argument('script', help='User script')
    parser.add_argument('args', nargs=argparse.REMAINDER,
                        help='User script parameters')
    options = parser.parse_args()

    common.process_default_options(options)

    logging.basicConfig(level=logging.DEBUG)

    logging.getLogger('pika').setLevel(logging.INFO)

    action_file = options.script
    logging.debug('loading file %s', action_file)
    import imp
    actions = imp.load_source('actions', action_file)

    try:
        # check if user function is defined
        assert hasattr(actions, 'gen_notification')
        # bring it on
        gen_notification = actions.gen_notification
    except AssertionError:
        print 'required function: gen_notification'
        print '  not found in module %s' % (action_file)
        raise SystemExit(1)

    LOG.debug("Start RabbitMQ CarSync server ...")

    LOG.debug("----------------------------------")

    # wrap user function as lambda to pass additional arguments
    if not handle(lambda: gen_notification(options.args)):
        sys.exit("error, bye")

    LOG.debug("...end.")


if __name__ == "__main__":
    main()
