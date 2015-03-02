/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "core_priv.h"
#include "logger.h"
#include "config.h"
#include "vbsclient.h"
#include "downloadmgr.h"
#include "rpmanager.h"
#include "filetools.h"
#include "strtools.h"
#include "dispatcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define CRITICAL_CHECK_STATUS(x) if (x == CORE_STATUS_CRITICAL) return CORE_STATUS_CRITICAL;
#define CRITICAL_CHECK_VOID(x) if (x == CORE_STATUS_CRITICAL) return;
#define AFTER_CRITICAL_CHECK_STATUS() if (!_configStorage) return CORE_STATUS_CRITICAL;

static int _autoUpgrade = 1;
static int _autoDownload = 1;
static unsigned int updateProgress = 0;
static CORE_STATUS_HANDLER _statusHandler = 0;
static config_Storage* _configStorage = 0;
static pthread_t _updateThread = 0;
static int _readyToDownload = 0;

static core_State _lastState = CORE_STATE_IDLE;
static core_Status _lastStatus = CORE_STATUS_OK;
static char* _lastMessage = 0;

typedef struct {
    char* updateFilesPath;
    proto_AvailableConfig* config;
} ConfigCache;
static ConfigCache _configCache;

static core_Status handleBrokenState() {
    // send error report
    vbsclient_sendErrorReport("Critical error occured! CIVG entered broken state!");

    // notify status handler if any
    if (_statusHandler) {
        _statusHandler(CORE_STATE_BROKEN, CORE_STATUS_CRITICAL);
    }

    // there is nothing more we can do, so cleanup
    core_cleanup();
    _lastStatus = CORE_STATUS_CRITICAL;
    return CORE_STATUS_CRITICAL;
}

static core_Status changeStatus(core_State newState, core_Status newStatus) {
    if (_lastState != newState || _lastStatus != newStatus) {
        // send update report if needed
        if (newState == CORE_STATE_UPGRADE) {
            vbsclient_sendUpdateReport(proto_UPDATE_PENDING, _configCache.config, "Starting upgrade.");
        } else  if (_lastState == CORE_STATE_UPGRADE) {
            if (newStatus == CORE_STATUS_OK) {
                vbsclient_sendUpdateReport(proto_UPDATE_SUCCESS, _configCache.config, "Upgrade finished.");
            } else {
                if (_lastMessage) {
                    char* buffer = strtools_sprintNew("Failed to perform upgrade! Last message: %s", _lastMessage);
                    if (buffer) {
                        vbsclient_sendUpdateReport(proto_UPDATE_FAILURE, _configCache.config, buffer);
                        free(buffer);
                    } else {
                        vbsclient_sendUpdateReport(proto_UPDATE_FAILURE, _configCache.config, "Failed to perform upgrade! Failed to allocate memory for last message.");
                    }
                } else {
                    vbsclient_sendUpdateReport(proto_UPDATE_FAILURE, _configCache.config, "Failed to perform upgrade!");
                }
            }
        }
        // notify status handler if any
        if (_statusHandler) {
            _statusHandler(newState, newStatus);
        }
        _lastState = newState;
        _lastStatus = newStatus;

        // write state to permanent storage
        if (config_setIValue(_configStorage, STATE_KEY, newState) != 0) {
            LOGM(LOGG_ERROR, "Can not save status to permanent storage! Can not ensure proper behaviour if status storage doesn't work!");
            return handleBrokenState();
        }
    }
    return CORE_STATUS_OK;
}

static void setupConfigCache(proto_AvailableConfig* config) {
    if (_configCache.config) {
        car_sync__proto__messages__rpcconfiguration_rsp__free_unpacked(_configCache.config, NULL);
    }
    _configCache.config = config;

    free(_configCache.updateFilesPath);
    if (_configCache.config && _configCache.config->update
        && _configCache.config->update->uuid
        && _configCache.config->update->to_version
        && _configCache.config->update->to_version->has_major_version
        && _configCache.config->update->to_version->has_minor_version
        && _configCache.config->update->to_version->has_build_version) {
        if (WORK_DIR[strlen(WORK_DIR) - 1] != '/') {
            _configCache.updateFilesPath = strtools_sprintNew("%s/%s/%d_%d_%d",WORK_DIR, _configCache.config->update->uuid,
                                                              _configCache.config->update->to_version->major_version,
                                                              _configCache.config->update->to_version->minor_version,
                                                              _configCache.config->update->to_version->build_version);
        } else {
            _configCache.updateFilesPath = strtools_sprintNew("%s%s/%d_%d_%d",WORK_DIR, _configCache.config->update->uuid,
                                                              _configCache.config->update->to_version->major_version,
                                                              _configCache.config->update->to_version->minor_version,
                                                              _configCache.config->update->to_version->build_version);
        }
    } else {
        _configCache.updateFilesPath = 0;
    }
}

static void updateConfigCache() {
    int bufferSize;
    void* buffer;
    if (config_getValue(_configStorage, AVAILABLE_CONFIG_KEY, &buffer, &bufferSize) == 0) {
        setupConfigCache(car_sync__proto__messages__rpcconfiguration_rsp__unpack(NULL, bufferSize, buffer));
        if (!_configCache.config) {
            LOGM(LOGG_WARNING, "Failed to unpack available config from storage!");
        }
        free(buffer);
    } else {
        setupConfigCache(0);
    }
}

static core_Status saveAvailableConfig(proto_AvailableConfig* availableConfig) {
    if (availableConfig == 0 && _configCache.config == 0) {
        // no action needed
        return 0;
    }

    // write config to permanent storage
    int status = 1;
    if (availableConfig) {
        size_t bufferSize = car_sync__proto__messages__rpcconfiguration_rsp__get_packed_size(availableConfig);
        void* buffer = malloc(bufferSize);
        if (buffer) {
            car_sync__proto__messages__rpcconfiguration_rsp__pack(availableConfig, buffer);
            status = config_setValue(_configStorage, AVAILABLE_CONFIG_KEY, buffer, bufferSize);
            free(buffer);
        }
    } else {
        status = config_removeKey(_configStorage, AVAILABLE_CONFIG_KEY);
    }
    if (status != 0) {
        LOGM(LOGG_ERROR, "Can not save available config to permanent storage! Can not ensure proper behaviour if config storage doesn't work!");
        return handleBrokenState();
    }
    updateConfigCache();
    return CORE_STATUS_OK;
}

static void clearLastMessage() {
    if (_lastMessage) {
        free(_lastMessage);
        _lastMessage = 0;
    }
}

static void updateNotificationHandler() {
    LOGM(LOGG_INFO, "Update notification received.");
    proto_InstalledConfig* installedConfig;
    if (rpmanager_getInstalledConfig(&installedConfig) == RPMANAGER_STATUS_OK) {
        LOGM(LOGG_INFO, "Checking available configuration.");
        vbsclient_checkConfig(installedConfig);
        car_sync__proto__messages__rpcconfiguration_req__free_unpacked(installedConfig, NULL);
    } else {
        LOGM(LOGG_ERROR, "Failed to get installed config!");
        handleBrokenState();
    }
}

static core_Status performUpgrade(int resume){
    CRITICAL_CHECK_STATUS(changeStatus(CORE_STATE_UPGRADE, CORE_STATUS_OK));
    clearLastMessage();
    rpmanager_Status rpStatus = (resume) ?
                rpmanager_resume(_configCache.config, _configCache.updateFilesPath, &_lastMessage):
                rpmanager_applyRP(_configCache.config, _configCache.updateFilesPath, &_lastMessage);
    switch (rpStatus) {
    case RPMANAGER_STATUS_OK:
        LOGM(LOGG_INFO, "Upgrade finished.");
        CRITICAL_CHECK_STATUS(changeStatus(CORE_STATE_IDLE, CORE_STATUS_OK));
        // check for more updates.
        updateNotificationHandler();
        break;
    case RPMANAGER_STATUS_CRITICAL:
        LOGM(LOGG_ERROR, "Critical error occured during upgrade!");
        return handleBrokenState();
    default:
        LOGM(LOGG_ERROR, "Failed to perform upgrade!");
        CRITICAL_CHECK_STATUS(changeStatus(CORE_STATE_IDLE, CORE_STATUS_ERROR));
        break;
    }
    CRITICAL_CHECK_STATUS(saveAvailableConfig(0));
    filetools_removeDir(WORK_DIR);
    clearLastMessage();
    return CORE_STATUS_OK;
}

static void downloadFinishedHandler(int status) {
    if (status == 0) {
        LOGM(LOGG_INFO, "Download finished.");
        if (_autoUpgrade) {
            LOGM(LOGG_INFO, "Starting upgrade.");
            CRITICAL_CHECK_VOID(performUpgrade(0));
        } else {
            LOGM(LOGG_INFO, "Waiting for upgrade trigger.");
            changeStatus(CORE_STATE_IDLE, CORE_STATUS_OK);
        }
    } else {
        LOGM(LOGG_WARNING, "Failed to download update files!");
        CRITICAL_CHECK_VOID(changeStatus(CORE_STATE_IDLE, CORE_STATUS_ERROR));
        CRITICAL_CHECK_VOID(saveAvailableConfig(0));
        filetools_removeDir(WORK_DIR);
    }
}

static core_Status applyConfig() {
    // _configCache.updateFilesPath should be != 0 and n_updates > 0 if there is something to download
    if (_configCache.updateFilesPath
        && _configCache.config->update->n_updates > 0) {
        proto_InstalledConfig* installedConfig;
        if (rpmanager_getInstalledConfig(&installedConfig) == RPMANAGER_STATUS_OK) {
            int isValid = 0;
            // check if version of installed RP  version match from_version
            for (size_t i = 0; i < installedConfig->n_rps; ++i) {
                if (installedConfig->rps[i]->uuid && _configCache.config->update->uuid
                    && strcmp(installedConfig->rps[i]->uuid, _configCache.config->update->uuid) == 0) {
                    if (proto_versionCmp(installedConfig->rps[i]->version, _configCache.config->update->from_version) == 0) {
                        isValid = 1;
                    }
                    break;
                }
            }
            car_sync__proto__messages__rpcconfiguration_req__free_unpacked(installedConfig, NULL);
            if (isValid) {
                if(0==_autoDownload) {
                    LOGM(LOGG_INFO, "New update available - waiting for download trigger.");
                    while(_readyToDownload!=1){
                        sleep(1);
                    }
                    _readyToDownload=0;
                } else {
                    LOGM(LOGG_INFO, "New update available - starting download.");
                }
                CRITICAL_CHECK_STATUS(changeStatus(CORE_STATE_DOWNLOAD, CORE_STATUS_OK));
                if (downloadmgr_downloadUFs(_configCache.config->update, _configCache.updateFilesPath) != 0) {
                    LOGM(LOGG_WARNING, "Failed to download update files!");
                    CRITICAL_CHECK_STATUS(changeStatus(CORE_STATE_IDLE, CORE_STATUS_ERROR));
                    CRITICAL_CHECK_STATUS(saveAvailableConfig(0));
                    filetools_removeDir(WORK_DIR);
                }

            } else {
                LOGM(LOGG_WARNING, "Received update doesn't match installed config!");
                CRITICAL_CHECK_STATUS(changeStatus(CORE_STATE_IDLE, CORE_STATUS_ERROR));
                CRITICAL_CHECK_STATUS(saveAvailableConfig(0));
            }
        } else {
            LOGM(LOGG_ERROR, "Failed to get installed config!");
            return handleBrokenState();
        }
    } else {
        LOGM(LOGG_INFO, "Nothing to upgrade.");
    }
    return CORE_STATUS_OK;
}

extern core_Status core_performDownload () {
    _readyToDownload=1;
    return CORE_STATUS_OK;

}

static void availableConfigHandler(proto_AvailableConfig* availableConfig) {
    LOGM(LOGG_INFO, "Received available config message.");
    // new config received before previous download end
    if (_lastState == CORE_STATE_DOWNLOAD) {
        int sameUpdate = _configCache.config && availableConfig
                && _configCache.config->update && availableConfig->update
                && _configCache.config->update->uuid && availableConfig->update->uuid
                && strcmp(_configCache.config->update->uuid, availableConfig->update->uuid) == 0
                && proto_versionCmp(_configCache.config->update->from_version, availableConfig->update->from_version) == 0
                && proto_versionCmp(_configCache.config->update->to_version, availableConfig->update->to_version) == 0;
        if (!sameUpdate) {
            // different update to be downloaded - cancel previous ones (if any)
            LOGM(LOGG_INFO, "More recent update found - cancel previous downloads.");
            downloadmgr_cancel();
            filetools_removeDir(WORK_DIR);
        } else {
            LOGM(LOGG_INFO, "Config message already handled - ignore this one.");
            // ignore available config message - it has been already handled
            return;
        }
    }

    CRITICAL_CHECK_VOID(saveAvailableConfig(availableConfig));
    CRITICAL_CHECK_VOID(applyConfig());
}

extern core_Status core_init(int autoUpgrade, int autoDownload) {
    LOG(LOGG_INFO, "Initializing CIVG (autoUpgrade=%d)", autoUpgrade);
    if (_configStorage) {
        LOGM(LOGG_ERROR, "Doubled CIVG initialization!");
        return CORE_STATUS_ERROR;
    }

    // basic setup
    _configStorage = config_newStorage(CORE_STORAGE_ID);
    if (!_configStorage) {
        LOGM(LOGG_ERROR, "Can not open config storage!");
        return CORE_STATUS_CRITICAL;
    }

    _lastState = CORE_STATE_IDLE;
    _lastStatus = CORE_STATUS_OK;
    _autoUpgrade = autoUpgrade;
    _autoDownload = autoDownload;
    if (vbsclient_init() != 0) {
        LOGM(LOGG_ERROR, "Failed to initialize VBS Client!");
        core_cleanup();
        return CORE_STATUS_CRITICAL;
    }
    vbsclient_setUpdateNotificationHandler(updateNotificationHandler);
    vbsclient_setAvailableConfigHandler(availableConfigHandler);
    if (rpmanager_init() != 0) {
        LOGM(LOGG_ERROR, "Failed to initialize RPManager!");
        return handleBrokenState();
    }
    if (downloadmgr_init() != 0) {
        LOGM(LOGG_ERROR, "Failed to initialize Download Manager!");
        return handleBrokenState();
    }
    downloadmgr_setFinishedHandler(downloadFinishedHandler);

    // get last saved status
    int restoredState = CORE_STATE_IDLE;
    config_getIValue(_configStorage, STATE_KEY, &restoredState);
    switch (restoredState) {
    case CORE_STATE_DOWNLOAD:
        LOGM(LOGG_INFO, "Resuming interrupted download.");
        // check if new updates appeared in the meantime.
        updateNotificationHandler();
        AFTER_CRITICAL_CHECK_STATUS();
        // and resume previous download in parallel (will be automatically
        // canceled if there is something newer)
        updateConfigCache();
        // _configCache.updateFilesPath should be != 0 if there is something to download
        if (_configCache.updateFilesPath) {
            CRITICAL_CHECK_STATUS(changeStatus(CORE_STATE_DOWNLOAD, CORE_STATUS_OK));
            if (downloadmgr_resume(_configCache.config->update, _configCache.updateFilesPath) != 0) {
                LOGM(LOGG_WARNING, "Failed to resume update files download!");
                CRITICAL_CHECK_STATUS(changeStatus(CORE_STATE_IDLE, CORE_STATUS_ERROR));
                CRITICAL_CHECK_STATUS(saveAvailableConfig(0));
                filetools_removeDir(WORK_DIR);
            }
        } else {
            LOGM(LOGG_WARNING, "Can not resume download - failed to get available config from storage!");
            CRITICAL_CHECK_STATUS(changeStatus(CORE_STATE_IDLE, CORE_STATUS_ERROR));
        }
        break;
    case CORE_STATE_UPGRADE:
        LOGM(LOGG_INFO, "Resuming interrupted upgrade.");
        updateConfigCache();
        if (_configCache.config) {
            CRITICAL_CHECK_STATUS(performUpgrade(1));
        } else {
            LOGM(LOGG_ERROR, "Can not resume upgrade - failed to get available config from storage!");
            return handleBrokenState();
        }
        break;
    default:
        // if no resume needed just check for updates on startup
        updateNotificationHandler();
        AFTER_CRITICAL_CHECK_STATUS();
        filetools_removeDir(WORK_DIR);
        break;
    }
    return _lastStatus;
}

extern void core_cleanup() {
    if (_configStorage) {
        _statusHandler = 0;
        downloadmgr_cleanup();
        rpmanager_cleanup();
        vbsclient_cleanup();

        setupConfigCache(0);
        clearLastMessage();

        config_deleteStorage(_configStorage);
        _configStorage = 0;
    }
}

static void* performUpgradeThread(void* tArgs)
{
    performUpgrade(0);
    pthread_exit(tArgs);
}

extern core_Status core_performUpgrade() {
    LOGM(LOGG_INFO, "Upgrade triggered.");
    if (!_configStorage
        || _lastState != CORE_STATE_IDLE
        || _lastStatus != CORE_STATUS_OK
        || !_configCache.config) {
        LOGM(LOGG_WARNING, "No update to perform!");
        return CORE_STATUS_ERROR;
    }
    core_Status status = CORE_STATUS_ERROR;
    if (_updateThread) {
        LOGM(LOGG_ERROR, "Doubled update thread initialization!");
    }
    pthread_attr_t update_attr;
    if (pthread_attr_init(&update_attr) == 0) {
        if (pthread_attr_setdetachstate(&update_attr, PTHREAD_CREATE_DETACHED) == 0) {
                if (pthread_create(&_updateThread, &update_attr, performUpgradeThread, NULL) != 0) {
                    LOGM(LOGG_ERROR, "Failed to create update thread!");
                } else {
                    status = CORE_STATUS_OK;
                }
        } else {
            LOGM(LOGG_ERROR, "Failed to set thread attributes!");
        }
        pthread_attr_destroy(&update_attr);
    } else {
        LOGM(LOGG_ERROR, "Failed to initialize thread attributes!");
    }

    return status;
}

extern void core_setStatusHandler(CORE_STATUS_HANDLER handler) {
    _statusHandler = handler;
}

extern void core_getStatus(core_State* state, core_Status* status)
{
    *state = _lastState;
    *status = _lastStatus;
}

extern void core_checkUpdates() {
    LOGM(LOGG_INFO, "update check triggered");
    core_State state;
    core_Status status;
    core_getStatus(&state,&status);
    if(state!=CORE_STATE_DOWNLOAD && state!=CORE_STATE_UPGRADE) {
        updateNotificationHandler();
    } else {
        LOGM(LOGG_DEBUG, "Update ongoing, update check unavailable");
    }
}

extern int core_getAvailableUpdates(char** uuid, char** version)
{
    int status = 1;

    do
    {
        if (_configCache.config == NULL){
            LOGM(LOGG_ERROR, "config = NULL.");
            break;
        }
        else if (_configCache.config->update == NULL)
        {
            LOGM(LOGG_ERROR, "No update set.");
            break;
        }
        else if (_configCache.config->update->uuid == NULL)
        {
            LOGM(LOGG_ERROR, "Can't get uuid - uuid = NULL.");
            break;
        }
        else if(_lastState == CORE_STATE_DOWNLOAD)
        {
            LOGM(LOGG_INFO, "Download in progress! No updates ready to perform yet!");
            break;
        }
        *uuid = strtools_sprintNew("%s", _configCache.config->update->uuid);
        if (_configCache.config->update->to_version == NULL)
        {
            LOGM(LOGG_ERROR, "Can't get version number.");
            free(*uuid);
            *uuid = NULL;
            *version = NULL;
            break;
        }
        *version = strtools_sprintNew("%d,%d,%d", _configCache.config->update->to_version->major_version, _configCache.config->update->to_version->minor_version,
                                                  _configCache.config->update->to_version->build_version);
        status = 0;
    } while (0);

    return status;
}
