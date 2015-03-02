/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "test_rpmanager.h"

static int (*_initFunc)() = 0;
static void (*_cleanupFunc)() = 0;
static rpmanager_Status (*_applyFunc)(const proto_AvailableConfig *, const char *, char **) = 0;
static rpmanager_Status (*_resumeFunc)(const proto_AvailableConfig *, const char *, char **) = 0;
static rpmanager_Status (*_installedConfigFunc)(proto_InstalledConfig **) = 0;

int rpmanager_init() {
    if (_initFunc) {
        return _initFunc();
    }
    return 0;
}

void rpmanager_cleanup() {
    if (_cleanupFunc) {
        _cleanupFunc();
    }
}

rpmanager_Status rpmanager_applyRP(const proto_AvailableConfig* availableConfig,
                                   const char* updateFilesPath, char** statusMessage) {
    if (_applyFunc) {
        return _applyFunc(availableConfig, updateFilesPath, statusMessage);
    }
    return RPMANAGER_STATUS_OK;
}

rpmanager_Status rpmanager_resume(const proto_AvailableConfig* availableConfig,
                                  const char* updateFilesPath, char** statusMessage) {
    if (_resumeFunc) {
        return _resumeFunc(availableConfig, updateFilesPath, statusMessage);
    }
    return RPMANAGER_STATUS_OK;
}

rpmanager_Status rpmanager_getInstalledConfig(proto_InstalledConfig** installedConfig) {
    if (_installedConfigFunc) {
        return _installedConfigFunc(installedConfig);
    }
    return RPMANAGER_STATUS_OK;
}

void testrpmanager_init(int (*func)())
{
    _initFunc = func;
}

void testrpmanager_cleanup(void (*func)())
{
    _cleanupFunc = func;
}

void testrpmanager_applyRP(rpmanager_Status (*func)(const proto_AvailableConfig *, const char *, char **))
{
    _applyFunc = func;
}

void testrpmanager_resume(rpmanager_Status (*func)(const proto_AvailableConfig *, const char *, char **))
{
    _resumeFunc = func;
}

void testrpmanager_getInstalledConfig(rpmanager_Status (*func)(proto_InstalledConfig **))
{
    _installedConfigFunc = func;
}
