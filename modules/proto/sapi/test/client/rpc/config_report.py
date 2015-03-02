import logging
import time
from southapi_pb2 import BaseMessage, RPC


QUEUE_NAME = 'RPC'
WAIT_FOR_RESPONSE = True

LOG = logging.getLogger('request')

def gen_request(args):
    LOG.debug('generate config request')

    LOG.debug(args)

    msg = BaseMessage()

    msg.VIN = 'SAJAC18R1AMV01091'
    msg.version.major_version = 0
    msg.version.minor_version = 1
    msg.version.build_version = 0

    msg.timestamp = str(time.time())
    msg.encryption_data.type = BaseMessage.Encryption.ENC_NONE
    msg.sig_data.type = BaseMessage.Signature.SIG_NONE

    r = RPC()
    swrp = r.conf_req.rps.add()
    swrp.uuid = 'c8a93b46-24fd-40f3-a3a5-b6e29879199f'
    swrp.version.major_version = 1
    swrp.version.minor_version = 0
    swrp.version.build_version = 0

    fwrp = r.conf_req.rps.add()
    fwrp.CopyFrom(swrp)
    fwrp.uuid = '4de32c79-53ef-49f7-bf20-0f3ee199eb60'

    msg.data = r.SerializeToString()

    serialized = msg.SerializeToString()
    LOG.debug('out RPC: %s', r)
    LOG.debug('out Message: %s', msg)
    LOG.debug('out msg: %r', serialized)
    LOG.debug('out msg len: %d', len(serialized))
    return serialized


def parse_resp(body):
    LOG.debug('parse config request response')
    msg = BaseMessage()

    LOG.info('--- Base message')
    BaseMessage.ParseFromString(msg, body)
    print msg

    LOG.info('--- RPC')
    rpc = RPC()
    RPC.ParseFromString(rpc, msg.data)
    print rpc

    if rpc.HasField('error'):
        print '--- ERROR'
        if rpc.error.reason:
            print '    reason:', rpc.error.reason
        else:
            print '    reason not specified'
    else:
        print '--- Got valid response'
