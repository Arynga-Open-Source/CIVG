/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * This is test downloader which mocks downloader bahaviour.
 */

#ifndef TEST_RPMANAGER_H
#define TEST_RPMANAGER_H

#include "rpmanager.h"

extern void testrpmanager_init(int (*func)());

extern void testrpmanager_cleanup(void (*func)());

extern void testrpmanager_applyRP(rpmanager_Status (*func)(const proto_AvailableConfig*,
                                                   const char*, char**));

extern void testrpmanager_resume(rpmanager_Status (*func)(const proto_AvailableConfig*,
                                              const char*, char**));

extern void testrpmanager_getInstalledConfig(rpmanager_Status (*func)(proto_InstalledConfig**));

#endif
