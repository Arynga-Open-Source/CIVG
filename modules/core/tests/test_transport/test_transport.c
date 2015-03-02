/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "test_transport.h"

static int (*_initFunc)() = 0;
static void (*_cleanupFunc)() = 0;
static int (*_sendFunc)(const void *, int) = 0;
static void (*_setHandlerFunc)(TRANSPORT_MSG_HANDLER) = 0;

int transport_init() {
    if (_initFunc) {
        return _initFunc();
    }
    return 0;
}

void transport_cleanup() {
    if (_cleanupFunc) {
        _cleanupFunc();
    }
}

int transport_sendMsg(const void* msg, int size) {
    if (_sendFunc) {
        return _sendFunc(msg, size);
    }
    return 0;
}

void transport_setMsgHandler(TRANSPORT_MSG_HANDLER handler) {
    if (_setHandlerFunc) {
        _setHandlerFunc(handler);
    }
}

void testtransport_init(int (*func)())
{
    _initFunc = func;
}


void testtransport_cleanup(void (*func)())
{
    _cleanupFunc = func;
}


void testtransport_sendMsg(int (*func)(const void *, int))
{
    _sendFunc = func;
}


void testtransport_setMsgHandler(void (*func)(TRANSPORT_MSG_HANDLER))
{
    _setHandlerFunc = func;
}
