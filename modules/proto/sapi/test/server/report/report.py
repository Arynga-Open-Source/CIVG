from southapi_pb2 import BaseMessage
import logging

LOG = logging.getLogger('report')

def handle_request(data, args):
    LOG.debug('got report')
