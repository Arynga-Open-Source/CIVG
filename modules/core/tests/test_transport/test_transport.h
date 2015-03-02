/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * This is test downloader which mocks downloader bahaviour.
 */

#ifndef TEST_TRANSPORT_H
#define TEST_TRANSPORT_H

#include "transport.h"

extern void testtransport_init(int (*func)());

extern void testtransport_cleanup(void (*func)());

extern void testtransport_sendMsg(int (*func)(const void* msg, int size));

extern void testtransport_setMsgHandler(void (*func)(TRANSPORT_MSG_HANDLER handler));

#endif
