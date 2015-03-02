#!/usr/bin/python
import sys
from os.path import basename

sys.path.append('../python')

import logging

from rmqwrap.client import Sink

LOG = logging.getLogger('rmq.drain')


def main():
    """
    """
    if len(sys.argv) < 2:
        print 'usage: %s <queue name>' % (basename(sys.argv[0]))
        raise SystemExit(1)

    logging.basicConfig(level=logging.DEBUG)

    logging.getLogger('pika').setLevel(logging.INFO)

    qname = sys.argv[1]

    sink = Sink('172.27.0.11', qname)
    sink.consume()
    sink.release()
    logging.info('closing...')


if __name__ == '__main__':
    main()
