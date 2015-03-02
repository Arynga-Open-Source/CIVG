/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "threaded_producer.h"
#include "test_config.h"

static int (*_initFunc)(const transport_config_ConnectionConfig *) = 0;
static void (*_cleanupFunc)() = 0;
static void (*_sendFunc)() = 0;
static int (*_pushFunc)(const void *, int) = 0;


int threaded_producer_init(const transport_config_ConnectionConfig* config) {
    if (_initFunc) {
        return _initFunc(config);
    }
    return 0;
}

void threaded_producer_cleanup() {
    if (_cleanupFunc) {
        _cleanupFunc();
    }
}

void threaded_producer_sendNext() {
    if (_sendFunc) {
        _sendFunc();
    }
}

int threaded_producer_pushMsg(const void* msg, int size) {
    if (_pushFunc) {
        return _pushFunc(msg, size);
    }
    return 0;
}

void test_producer_init(int (*func)(const transport_config_ConnectionConfig *))
{
    _initFunc = func;
}

void test_producer_cleanup(void (*func)())
{
    _cleanupFunc = func;
}


void test_producer_sendNext(void (*func)())
{
    _sendFunc = func;
}

void test_producer_pushMsg(int (*func)(const void *, int))
{
    _pushFunc = func;
}
