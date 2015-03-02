'''Common routines related to RMQ'''

import pika
import logging

LOG = logging.getLogger('rmq.common')

QUEUE_NAME_SUFFIX = ''

def new_connection(host=None, **args):
    """obtain a new blocking connection with given pika.ConnectionParameters

    Arguments:
    - `**args`:
    """
    assert host != None

    params = pika.ConnectionParameters(host=host, **args)
    LOG.debug('conection parameters:')
    LOG.debug('  host: %s', host)

    LOG.debug('connect')
    try:
        connection = pika.BlockingConnection(params)
    except pika.exceptions.AMQPConnectionError, err:
        LOG.error('error connecting to RMQ')
        LOG.error('  reason: %s', repr(err))
        connection = None

    return connection


def get_queue(channel, name, durable=False, **args):
    """Declare a named queue"""
    return channel.queue_declare(queue=name,
                                 durable=durable,
                                 **args)


def clear_queue(channel, name):
    """Purge all messages in queue

    Arguments:
    - `channel`:
    - `name`:
    """
    LOG.debug('purging queue %s', name)
    return channel.queue_purge(name)


def delete_queue(channel, name):
    """Delete queue of given name

    Arguments:
    - `channel`:
    - `name`:
    """
    LOG.debug('delete queue %s', name)
    return channel.queue_delete(queue=name)


def get_anon_queue(channel, **args):
    """Declare an anonymous queue"""
    return channel.queue_declare(**args)


def send_to_queue(channel, to_name, body='', exchange='', **args):
    """Send message to queue"""
    channel.basic_publish(exchange=exchange,
                          routing_key=to_name,
                          body=body,
                          **args)

def singleton(cls):
    """
    signleton decorator function
    """
    instances = {}
    def getinstance():
        if cls not in instances:
            instances[cls] = cls()
        return instances[cls]
    return getinstance


def add_default_options(parser):
    """Add default options to option parser
    """
    parser.add_argument('-d', '--debug', dest='debug',
                        help='enable debugs', action='store_true',
                        default=False)
    parser.add_argument('-s', '--queue-suffix', dest='queue_suffix',
                        help='queue name suffix')
    return parser


def process_default_options(options):
    """Process common options"""
    if options.queue_suffix:
        global QUEUE_NAME_SUFFIX
        QUEUE_NAME_SUFFIX = options.queue_suffix


def gen_queue_name(name):
    """Return RMQ queue name accounting for suffix"""
    return name + QUEUE_NAME_SUFFIX
