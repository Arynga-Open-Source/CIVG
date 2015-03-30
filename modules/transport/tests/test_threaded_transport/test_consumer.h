/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#ifndef TEST_CONSUMER_H
#define TEST_CONSUMER_H

#include "threaded_consumer.h"

void test_consumer_init(int (*func)(const transport_config_ConnectionConfig* config));

void test_consumer_cleanup(void (*func)());

void test_consumer_receiveNext(unsigned int (*func)(int timeout));

void test_consumer_break(void (*func)());

void test_consumer_popMsg(int (*func)(void** msg, int* size));

#endif
