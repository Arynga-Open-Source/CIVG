#!/usr/bin/python
import sys
sys.path.append('../python')

from os.path import basename
#from optparse import OptionParser
import argparse
import logging

try:
    from southapi_pb2 import BaseMessage
except ImportError as e:
    print '''
Error: failed to import message definitions
       perhaps you need to build python bindings??
'''
    print str(e)
    sys.exit(1)

from rmqwrap.client import CarSyncReq
from rmqwrap import common


def main():
    """
    """
    # parser = OptionParser('usage: %prog [options] <script> [<script params>]')
    parser = argparse.ArgumentParser(description='Execute South API client actions')
    parser = common.add_default_options(parser)
    parser.add_argument('script', help='Client script')
    parser.add_argument('args', nargs=argparse.REMAINDER,
                        help='Client script parameters')
    # (options, args) = parser.parse_args()
    args = parser.parse_args()
    common.process_default_options(args)

    action_file = args.script

    logging.basicConfig(level=logging.DEBUG)

    logging.getLogger('pika').setLevel(logging.INFO)

    logging.debug('import actions file: %s', action_file)
    import imp
    actions = imp.load_source('actions', action_file)

    try:
        assert hasattr(actions, 'gen_request')
        assert hasattr(actions, 'parse_resp')
        assert hasattr(actions, 'QUEUE_NAME')
    except AssertionError:
        print 'required functions: gen_request, parse_resp'
        print '  not found in module %s' % (action_file)
        raise SystemExit(1)

    request = actions.gen_request(args.args)
    logging.debug('request size: %d bytes', len(request))

    req = CarSyncReq('172.27.0.11')

    expecting_response = getattr(actions, 'WAIT_FOR_RESPONSE', True)

    response = req.call(common.gen_queue_name(actions.QUEUE_NAME), request,
                        expecting_response)
    if expecting_response:
        if response:
            logging.debug('response size: %d bytes', len(response))
            actions.parse_resp(response)
        else:
            logging.error('no response received')
    else:
        logging.info('not expecting any response')

    logging.info('closing...')
    req.release()


if __name__ == '__main__':
    main()
