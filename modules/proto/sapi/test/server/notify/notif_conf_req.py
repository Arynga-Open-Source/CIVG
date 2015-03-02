import logging
import time
from southapi_pb2 import BaseMessage, Notification

LOG = logging.getLogger('notification')

def gen_notification(args):

    LOG.debug('.. generate notification')
    msg = BaseMessage()

    LOG.debug('')
    msg.VIN = ''
    msg.timestamp = str(time.time())
    msg.encryption_data.type = BaseMessage.Encryption.ENC_NONE
    msg.sig_data.type = BaseMessage.Signature.SIG_NONE

    LOG.debug('base message: %s', msg)

    notif = Notification()
    notif.type = Notification.CONF_REPORT

    LOG.debug('notification message: %s', notif)
    msg.data = notif.SerializeToString()

    return msg.SerializeToString()
