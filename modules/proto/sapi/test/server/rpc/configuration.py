from southapi_pb2 import BaseMessage, RPC

import time
import logging

CNT = 0
SIMULATE_ERRORS = 0

LOG = logging.getLogger('rpc-configuration')

def handle_request(body, args):
    # request counter
    global CNT
    CNT += 1

    LOG.debug('parse BaseMessage')
    LOG.debug('body: %r', body)
    LOG.debug('body len: %d', len(body))

    msg = BaseMessage()
    # parse incoming message
    BaseMessage.ParseFromString(msg, body)
    LOG.debug("base message: %s", msg)

    # we're dealing with RPC messages
    rpc = RPC()
    LOG.debug('parse RPC message')
    RPC.ParseFromString(rpc, msg.data)

    # check if it's a configuration request, note that fields are
    # marked as optional
    LOG.debug('rpc: %s', rpc)
    if not rpc.HasField('conf_req'):
        LOG.error('not a RPC request')
        return

    # pick configuration request part
    req = rpc.conf_req
    if len(req.rps) == 0:
        # we'd expect the client to send at least one RP
        LOG.error('no RPs in the list')
        return

    # use the first RP from the list
    rp = req.rps[0]
    LOG.debug('current version of RP %s is %d.%d.%d',
              rp.uuid,
              rp.version.major_version,
              rp.version.minor_version,
              rp.version.build_version)

    resp = BaseMessage()
    resp.version.major_version = 0
    resp.version.minor_version = 1
    resp.version.build_version = 0
    resp.timestamp = str(time.time())

    rrpc = RPC()

    # prepare example response message
    # - use previous RP UUID
    # - bump build version
    # - add fake UF

    if SIMULATE_ERRORS and \
       (self._cnt % SIMULATE_ERRORS) == 0:
        LOG.info("-- simulating error")
        rrpc.error.reason = 'simulated error'
        rrpc.error.code = '1337'
    else:
        # configuration response
        rsp = rrpc.conf_rsp

        # fill out update field
        update_desc = rsp.update

        # use previous uuid
        update_desc.uuid = rp.uuid
        update = update_desc.updates.add()
        update.uuid = '366a924f-5ca5-4f3f-8b9e-95cb7da313c2'
        update.url = 'http://localhost:8080/366a924f-5ca5-4f3f-8b9e-95cb7da313c2.uf'

        update_desc.from_version.CopyFrom(rp.version)
        update_desc.to_version.CopyFrom(rp.version)
        update_desc.to_version.build_version += 1

    resp.VIN = msg.VIN
    resp.data = rrpc.SerializeToString()

    return resp.SerializeToString()

