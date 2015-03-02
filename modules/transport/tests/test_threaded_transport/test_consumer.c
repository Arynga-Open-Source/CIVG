/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "threaded_consumer.h"
#include "test_config.h"
#include "logger.h"
#include <pthread.h>

static int (*_initFunc)(const transport_config_ConnectionConfig *) = 0;
static void (*_cleanupFunc)() = 0;
static unsigned int (*_recFunc)() = 0;
static void (*_breakFunc)() = 0;
static int (*_popFunc)(void **, int *) = 0;

int threaded_consumer_init(const transport_config_ConnectionConfig* config) {
    if (_initFunc) {
        return _initFunc(config);
    }
    return 0;
}

void threaded_consumer_cleanup() {
    if (_cleanupFunc) {
        _cleanupFunc();
    }
}

unsigned int threaded_consumer_receiveNext() {
    if (_recFunc) {
        return _recFunc();
    }
    return 0;
}

void threaded_consumer_break() {
    if (_breakFunc) {
        _breakFunc();
    }
}

int threaded_consumer_popMsg(void** msg, int* size) {
    if (_popFunc) {
        return _popFunc(msg, size);
    }
    return 0;
}

void test_consumer_init(int (*func)(const transport_config_ConnectionConfig *))
{
    _initFunc = func;
}

void test_consumer_cleanup(void (*func)())
{
    _cleanupFunc = func;
}

void test_consumer_receiveNext(unsigned int (*func)(int))
{
    _recFunc = func;
}

void test_consumer_break(void (*func)())
{
    _breakFunc = func;
}

void test_consumer_popMsg(int (*func)(void **, int *))
{
    _popFunc = func;
}
