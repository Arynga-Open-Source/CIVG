#!/usr/bin/python
import sys
from os.path import basename

sys.path.append('../python')

import logging

from rmqwrap.common import new_connection, delete_queue

LOG = logging.getLogger('rmq.delete')


def main():
    """
    """
    if len(sys.argv) < 2:
        print 'usage: %s <queue name>' % (basename(sys.argv[0]))
        raise SystemExit(1)

    logging.basicConfig(level=logging.DEBUG)

    logging.getLogger('pika').setLevel(logging.INFO)

    qname = sys.argv[1]

    connection = new_connection('172.27.0.11')
    channel = connection.channel()

    logging.info('delete queue %s', qname)
    delete_queue(channel, qname)

    logging.info('closing...')
    connection.close()



if __name__ == '__main__':
    main()
