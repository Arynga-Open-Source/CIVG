/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#define LOGGER_MODULE "CIVG_RPMANAGER"

#include "rpmanager_priv.h"
#include "imagehandler.h"
#include "strtools.h"
#include "config.h"
#include "logger.h"
#include "filetools.h"
#include <stdlib.h>
#include <string.h>

static config_Storage* _configStorage = 0;
static rpmanager_State _state = RPMANAGER_STATE_UPDATE_SUCCESS;
static proto_InstalledConfig* _installedConfig = 0;
static char* _lastMessage = 0;
static char* _imagehandlerMessage = 0;

typedef enum {
    RPMANAGER_ACTION_NORMAL_UPDATE,
    RPMANAGER_ACTION_RESUME_UPDATE
} rpmanager_Action;

static void updateLastMessage(char* msg) {
    free(_lastMessage);
    _lastMessage = msg;
}

static void appendToLastMessage(char* msg)
{
    char* newMessage = strtools_sprintNew("%s %s", _lastMessage, msg);
    free(_lastMessage);
    free(msg);
    _lastMessage = newMessage;
}

static void appendImagehandlerMessage() {
    if (_imagehandlerMessage)
    {
        char* newMessage = strtools_sprintNew("%s *** Error message from imagehandler: %s", _lastMessage, _imagehandlerMessage);
        free(_lastMessage);
        free(_imagehandlerMessage);
        _lastMessage = newMessage;
        _imagehandlerMessage = 0;
    }
}

static void setStatusMessage(char** statusMessage) {
    *statusMessage = _lastMessage;
    _lastMessage = 0;
}

static int saveState(rpmanager_State state) {
    _state = state;
    if (config_setIValue(_configStorage, RPMANAGER_STATE_KEY, _state)) {
        updateLastMessage(strtools_sprintNew("Cannot save state to permanent storage! State: %d", state));
        LOGM(LOGG_ERROR, _lastMessage);
        return 1;
    }
    return 0;
}

static int restoreState() {
    if (config_getIValue(_configStorage, RPMANAGER_STATE_KEY, (int*)&_state))
    {
        updateLastMessage(strtools_sprintNew("Cannot restore state from permanent storage!"));
        LOGM(LOGG_ERROR, _lastMessage);
        return 1;
    }
    return 0;
}

static proto_ReleasePackageMeta* findInstalledRPById(const char* uuid)
{
    if (uuid) {
        for (size_t i = 0; i < _installedConfig->n_rps; ++i)
        {
            if(_installedConfig->rps[i]->uuid
               && strcmp(_installedConfig->rps[i]->uuid, uuid) == 0)
                return _installedConfig->rps[i];
        }
    }
    return 0;
}

/* Updates installed config after successful RP installation */
static int updateInstalledConfig(proto_UpdateDescriptor* update)
{
    proto_ReleasePackageMeta* updatedRP = findInstalledRPById(update->uuid);
    // if preActionChecks passed then it can't be NULL
    updatedRP->version->has_major_version = update->to_version->has_major_version;
    updatedRP->version->major_version = update->to_version->major_version;
    updatedRP->version->has_minor_version = update->to_version->has_minor_version;
    updatedRP->version->minor_version = update->to_version->minor_version;
    updatedRP->version->has_build_version = update->to_version->has_build_version;
    updatedRP->version->build_version = update->to_version->build_version;

    // write config to permanent storage
    int status = 1;
    size_t bufferSize = car_sync__proto__messages__rpcconfiguration_req__get_packed_size(_installedConfig);
    void* buffer = malloc(bufferSize);
    if (buffer) {
        car_sync__proto__messages__rpcconfiguration_req__pack(_installedConfig, buffer);
        status = config_setValue(_configStorage, INSTALLED_CONFIG_KEY, buffer, bufferSize);
        free(buffer);
    }
    if (status != 0) {
        updateLastMessage(strtools_sprintNew("Can not save installed config to permanent storage!"));
        LOGM(LOGG_ERROR, _lastMessage);
        return 1;
    }
    return status;
}

/* Restores installed config from storage */
static int restoreInstalledConfig()
{
    if (_installedConfig) {
        car_sync__proto__messages__rpcconfiguration_req__free_unpacked(_installedConfig, NULL);
    }
    if (rpmanager_getInstalledConfig(&_installedConfig) != RPMANAGER_STATUS_OK) {
        _installedConfig = 0;
        return 1;
    }
    return 0;
}

/* Checks if current requested action is consistent with current RPManager state */
static int checkStateConsistency(rpmanager_Action action)
{
    /* Check consistency of RPManager state and requested action */
    if (action==RPMANAGER_ACTION_NORMAL_UPDATE) {
        if ((_state==RPMANAGER_STATE_UPDATE_PENDING) || (_state==RPMANAGER_STATE_ROLLBACK_PENDING)) {
            updateLastMessage(strtools_sprintNew("Cannot run update with inconsistent state! "
                                                 "Try resuming previous update first."));
            LOGM(LOGG_ERROR, _lastMessage);
            return 1;
        }
    }
    else if (action==RPMANAGER_ACTION_RESUME_UPDATE) {
        if  ((_state==RPMANAGER_STATE_UPDATE_SUCCESS) || (_state==RPMANAGER_STATE_ROLLBACK_SUCCESS)) {
            updateLastMessage(strtools_sprintNew("There is nothing to resume, "
                                                 "previous update was either successful or completely rolled back."));
            LOGM(LOGG_WARNING, _lastMessage);
            return 1;
        }
    }
    return 0;
}

static const proto_ReleasePackageMeta* findRPById(const proto_AvailableConfig* availableConfig, const char* uuid)
{
    if (uuid) {
        for (size_t i = 0; i < availableConfig->n_rps; ++i)
        {
            if(availableConfig->rps[i]->uuid
               && strcmp(availableConfig->rps[i]->uuid, uuid) == 0)
                return availableConfig->rps[i];
        }
    }
    return 0;
}

static int checkUpdateConsistency(const proto_AvailableConfig* availableConfig)
{
    if (!availableConfig
        || !availableConfig->update
        || availableConfig->update->n_updates == 0) {

        updateLastMessage(strtools_sprintNew("No updates were found in available config."));
        LOGM(LOGG_WARNING, _lastMessage);
        return 1;
    }

    int status = 1;
    // number of assigned and installed rps match
    if (_installedConfig->n_rps == availableConfig->n_rps) {
        status = 0;
        // and all uuids match as well
        for (size_t i = 0; status == 0 && i < _installedConfig->n_rps; ++i) {
            if (findRPById(availableConfig, _installedConfig->rps[i]->uuid) == 0) {
                status = 1;
                break;
            }
        }
    }
    if (status != 0) {
        updateLastMessage(strtools_sprintNew("Available config does not match installed one!"));
        LOGM(LOGG_WARNING, _lastMessage);
        return 1;
    }

    const proto_ReleasePackageMeta* rpMeta = findInstalledRPById(availableConfig->update->uuid);
    if (!rpMeta)
    {
        updateLastMessage(strtools_sprintNew("Inconsistent config - update RP uuid not found in assigned RPs! UpdateUUID: %s", availableConfig->update->uuid));
        LOGM(LOGG_WARNING, _lastMessage);
        return 1;
    }
    // Check if "from" versions match installed one
    if(proto_versionCmp(rpMeta->version, availableConfig->update->from_version) != 0)
    {
        updateLastMessage(strtools_sprintNew("Installed package version and update \"from\" version do not match!"));
        LOGM(LOGG_WARNING, _lastMessage);
        return 1;
   }

    rpMeta = findRPById(availableConfig, availableConfig->update->uuid);
    // does not have to check rp != 0 - if it was found in installed config it must be in available as well at this point (installed match available)
    // Check if "to" versions is valid
   if(proto_versionCmp(availableConfig->update->to_version, availableConfig->update->from_version) != 1
      || proto_versionCmp(availableConfig->update->to_version, rpMeta->version) == 1)
   {
       updateLastMessage(strtools_sprintNew("Invalid update \"to\" version!"));
       LOGM(LOGG_WARNING, _lastMessage);
       return 1;
   }

   return 0;
}

static char* buildImageDir(const char* ufsPath, const char* ufUuid) {
    char* imageDir = filetools_joinPath(ufsPath, ufUuid);
    if (!imageDir) {
        updateLastMessage(strtools_sprintNew("Cannot allocate memory for image dir!"));
        LOGM(LOGG_ERROR, _lastMessage);
    }
    return imageDir;
}

static int runRollback(const proto_AvailableConfig* availableConfig,
                       const char* updateFilesPath, int startingUfNb)
{
    if (saveState(RPMANAGER_STATE_ROLLBACK_PENDING) != 0) {
        return 1;
    }
    int status = 0;
    char* imageDir = 0;
    proto_UpdateFileMeta* ufMeta = 0;

    for (int i = startingUfNb; i >=0 ; --i)
    {
        imageDir = buildImageDir(updateFilesPath, availableConfig->update->updates[i]->uuid);
        if (!imageDir) {
            status = 1;
            break;
        }

        ufMeta = proto_unpackMetaFile(imageDir);

        if (!ufMeta) {
            appendToLastMessage(strtools_sprintNew("Failed to read UF meta file!"));
            LOGM(LOGG_ERROR, _lastMessage);
            status = 1;
            break;
        }

        LOG(LOGG_DEBUG, "Rolling-back image from: %s", imageDir);
        if (imagehandler_rollback(imageDir, ufMeta->type, &_imagehandlerMessage) != 0)
        {
            appendToLastMessage(strtools_sprintNew("Failed to rollback image! UUID: %s",
                                                   availableConfig->update->updates[i]->uuid));
            appendImagehandlerMessage();
            LOGM(LOGG_ERROR, _lastMessage);
            status = 1;
            break;
        }
        //save last installed uf nb
        if (config_setIValue(_configStorage, LAST_INSTALLED_UF_NB_KEY, i - 1) != 0)
        {
            appendToLastMessage(strtools_sprintNew("Cannot write last installed UF to permanent storage!"));
            LOGM(LOGG_ERROR, _lastMessage);
            status = 1;
            break;
        }

        free(imageDir);
        imageDir=NULL;
        car_sync__proto__release_package__update_file_meta__free_unpacked(ufMeta, NULL);
        ufMeta=NULL;
    }

    if(ufMeta) {
        car_sync__proto__release_package__update_file_meta__free_unpacked(ufMeta, NULL);
    }

    free(imageDir);

    if (status == 0) {
        config_removeKey(_configStorage, LAST_INSTALLED_UF_NB_KEY);
        LOGM(LOGG_DEBUG, "Rollback successful");
        if (saveState(RPMANAGER_STATE_ROLLBACK_SUCCESS) != 0) {
            return 1;
        }
    }
    return status;
}

/* Starts installing update files starting from the uf with index given by "startingUFNb" */
static int runUpdate(const proto_AvailableConfig* availableConfig,
                     const char* updateFilesPath, int startingUFNb)
{
    if (saveState(RPMANAGER_STATE_UPDATE_PENDING) != 0) {
        return 1;
    }
    int status = 0;
    char* imageDir = 0;
    proto_UpdateFileMeta* ufMeta = 0;
    size_t i;
    for (i = startingUFNb; i < availableConfig->update->n_updates; ++i)
    {
        imageDir = buildImageDir(updateFilesPath, availableConfig->update->updates[i]->uuid);
        if (!imageDir) {
            status = 1;
            break;
        }

        ufMeta = proto_unpackMetaFile(imageDir);

        if (!ufMeta) {
            updateLastMessage(strtools_sprintNew("Failed to read UF meta file!"));
            LOGM(LOGG_ERROR, _lastMessage);
            status = 1;
            break;
        }

        // obtain file name from url
        // TODO: think about unified data name, for example image.dat
        char* fName = strrchr(ufMeta->url, '/');
        if (!fName || strlen(++fName) == 0) {
            updateLastMessage(strtools_sprintNew("Cannot obtain data file name for update %s!", ufMeta->uuid));
            LOGM(LOGG_ERROR, _lastMessage);
            status = -1;
            break;
        }

        char* dataPath = filetools_joinPath(imageDir,"data");
        char* fullPath = filetools_joinPath(dataPath, fName);

        LOG(LOGG_DEBUG, "Verifying checksums for %s\n", fullPath);
        char* checksum = filetools_getSHA1Checksum(fullPath);
        free(dataPath);
        free(fullPath);

        if(!checksum){
            updateLastMessage(strtools_sprintNew("Failed to calculate checksum for image!"));
            LOGM(LOGG_ERROR, _lastMessage);
            status = -1;
            break;
        }
        else if(strcmp(ufMeta->checksum, checksum) != 0){
            updateLastMessage(strtools_sprintNew("Calculated and expected checksums does not match each other for image %s.", imageDir));
            LOGM(LOGG_ERROR, _lastMessage);
            free(checksum);
            status = -1;
            break;
        }

        LOGM(LOGG_DEBUG, "Checksum ok!");
        free(checksum);

        LOG(LOGG_DEBUG, "Applying image from: %s",imageDir);
        if (imagehandler_apply(imageDir, ufMeta->type, &_imagehandlerMessage) != 0) {
            updateLastMessage(strtools_sprintNew("Failed to apply image! UUID: %s", availableConfig->update->updates[i]->uuid));
            appendImagehandlerMessage();
            status = -1;
            break;
        }
        //save last installed uf nb
        if (config_setIValue(_configStorage, LAST_INSTALLED_UF_NB_KEY, i) != 0)
        {
            updateLastMessage(strtools_sprintNew("Cannot write last installed UF to permanent storage!"));
            LOGM(LOGG_ERROR, _lastMessage);
            status = 1;
            break;
        }

        free(imageDir);
        imageDir=NULL;

        car_sync__proto__release_package__update_file_meta__free_unpacked(ufMeta, NULL);
        ufMeta=NULL;
    }

    if(ufMeta) {
        car_sync__proto__release_package__update_file_meta__free_unpacked(ufMeta, NULL);
    }
    free(imageDir);

    if (status == 0) {
        if (saveState(RPMANAGER_STATE_UPDATE_SUCCESS) != 0) {
            return 1;
        }
    } else if (status == -1) {
        LOGM(LOGG_WARNING, "Failed to perform update - rollback needed!");
        status = runRollback(availableConfig, updateFilesPath, i);
    }

    return status;
}

/* Starts rolling back update files starting from the uf with index given by "starting_uf_nb" */
/* Runs recover script for the uf with index given by "uf_nb" */
static int runRecover(const proto_AvailableConfig* availableConfig, const char* updateFilesPath, int ufNb)
{
    char* imageDir = 0;

    imageDir = buildImageDir(updateFilesPath, availableConfig->update->updates[ufNb]->uuid);
    if (!imageDir) {
        return 1;
    }

    int status = 1;
    proto_UpdateFileMeta* ufMeta = proto_unpackMetaFile(imageDir);
    if (ufMeta) {
        LOG(LOGG_DEBUG, "Recovering image from: %s", imageDir);
        status = imagehandler_resume(imageDir, ufMeta->type, &_imagehandlerMessage );
        if (status != 0) {
            updateLastMessage(strtools_sprintNew("Failed to recover image! UUID: %s", availableConfig->update->updates[ufNb]->uuid));
            appendImagehandlerMessage();
            LOGM(LOGG_ERROR, _lastMessage);
        }

        car_sync__proto__release_package__update_file_meta__free_unpacked(ufMeta, NULL);
    } else {
        updateLastMessage(strtools_sprintNew("Failed to read UF meta file!"));
        LOGM(LOGG_ERROR, _lastMessage);
    }
    free(imageDir);

    return status;
}

static rpmanager_Status preActionChecks(const proto_AvailableConfig* availableConfig,
                                        rpmanager_Action action)
{
    if (restoreState() != 0) {
        return RPMANAGER_STATUS_CRITICAL;
    }

    if (restoreInstalledConfig()) {
        return RPMANAGER_STATUS_CRITICAL;
    }

    if (checkStateConsistency(action) != 0
        || checkUpdateConsistency(availableConfig) != 0) {
        return RPMANAGER_STATUS_ERROR;
    }
    return RPMANAGER_STATUS_OK;
}

static rpmanager_Status finalizeAction(const proto_AvailableConfig* availableConfig)
{
    if (_state == RPMANAGER_STATE_UPDATE_SUCCESS) {
        //RP installed successfully
        if (config_removeKey(_configStorage, LAST_INSTALLED_UF_NB_KEY) != 0)
        {
            updateLastMessage(strtools_sprintNew("Cannot save last installed UF to permanent storage!"));
            LOGM(LOGG_ERROR, _lastMessage);
            return RPMANAGER_STATUS_CRITICAL;
        }

        if (updateInstalledConfig(availableConfig->update) != 0) {
            updateLastMessage(strtools_sprintNew("Cannot update installed config and save to permanent storage!"));
            LOGM(LOGG_ERROR, _lastMessage);
            return RPMANAGER_STATUS_CRITICAL;
        }
        LOGM(LOGG_DEBUG, "Release package installed successfully.");
        return RPMANAGER_STATUS_OK;
    } else if (_state == RPMANAGER_STATE_ROLLBACK_SUCCESS) {
        //RP not installed but recover and rollback performed successfully
        updateLastMessage(strtools_sprintNew("Failed to install release package, but rollback was successful."));
        LOGM(LOGG_WARNING, _lastMessage);
        return RPMANAGER_STATUS_ERROR;
    }
    // this should never happen
    updateLastMessage(strtools_sprintNew("Unexpected RPManager state!"));
    LOGM(LOGG_ERROR, _lastMessage);
    return RPMANAGER_STATUS_CRITICAL;
}

/* Main function, performs normal update or resumes update based on variable "action" */
static rpmanager_Status runAction(const proto_AvailableConfig* availableConfig,
                            const char* updateFilesPath, rpmanager_Action action)
{
    rpmanager_Status status = preActionChecks(availableConfig, action);
    if (status != RPMANAGER_STATUS_OK) {
        return status;
    }

    // Perform clean update if RPManager state is consistent
    //this is equivalent to (action == RPMANAGER_ACTION_NORMAL_UPDATE)
    if  (_state == RPMANAGER_STATE_UPDATE_SUCCESS
         || _state == RPMANAGER_STATE_ROLLBACK_SUCCESS) {
        if(runUpdate(availableConfig, updateFilesPath, 0) != 0) {
            return RPMANAGER_STATUS_CRITICAL;
        }
    } else if (_state == RPMANAGER_STATE_UPDATE_PENDING) {
        // If the state is not consistent (last update was interrupted), perform recover and continue updating
        // check where we have finished
        int ufNb;
        if (config_getIValue(_configStorage, LAST_INSTALLED_UF_NB_KEY, &ufNb) != 0) {
            updateLastMessage(strtools_sprintNew("Cannot get last installed UF from permanent storage!"));
            LOGM(LOGG_ERROR, _lastMessage);
            return RPMANAGER_STATUS_CRITICAL;
        }

        // Perform recover on UF which whose installation was interrupted
        if(runRecover(availableConfig, updateFilesPath, ++ufNb) != 0) {
            return RPMANAGER_STATUS_CRITICAL;
        }

        // Continue update
        if (runUpdate(availableConfig, updateFilesPath, ++ufNb) != 0) {
            return RPMANAGER_STATUS_CRITICAL;
        }
    } else if (_state==RPMANAGER_STATE_ROLLBACK_PENDING) {
        // If the state is not consistent (last rollback was interrupted), perform recover and continue rolling back
        // check where we have finished
        int ufNb;
        if (config_getIValue(_configStorage, LAST_INSTALLED_UF_NB_KEY, &ufNb) != 0) {
            updateLastMessage(strtools_sprintNew("Cannot get last installed UF from permanent storage!"));
            LOGM(LOGG_ERROR, _lastMessage);
            return RPMANAGER_STATUS_CRITICAL;
        }

        /* Perform recover on UF which whose rollback was interrupted */
        if(runRecover(availableConfig, updateFilesPath, ufNb) != 0) {
            return RPMANAGER_STATUS_CRITICAL;
        }

        /* Continue rollback which was interrupted */
        if (ufNb > 0) {
            if (runRollback(availableConfig, updateFilesPath, --ufNb) != 0)
                return RPMANAGER_STATUS_CRITICAL;
        } else {
            //UF number 0 was recovered so rollback was successful
            if (saveState(RPMANAGER_STATE_ROLLBACK_SUCCESS) != 0) {
                return RPMANAGER_STATUS_CRITICAL;
            }
        }
    }

    return finalizeAction(availableConfig);
}

rpmanager_Status rpmanager_getInstalledConfig(proto_InstalledConfig** installedConfig)
{
    int bufferSize;
    void* buffer;
    if (config_getValue(_configStorage, INSTALLED_CONFIG_KEY, &buffer, &bufferSize) == 0) {
        *installedConfig = car_sync__proto__messages__rpcconfiguration_req__unpack(NULL, bufferSize, buffer);
        free(buffer);

        if (!(*installedConfig)) {
            updateLastMessage(strtools_sprintNew("Failed to unpack installed config from storage!"));
            LOGM(LOGG_ERROR, _lastMessage);
            return RPMANAGER_STATUS_CRITICAL;
        }
    } else {
        updateLastMessage(strtools_sprintNew("Cannot read installed config from permanent storage!"));
        LOGM(LOGG_ERROR, _lastMessage);
        return RPMANAGER_STATUS_CRITICAL;
    }
    return RPMANAGER_STATUS_OK;
}

rpmanager_Status rpmanager_applyRP(const proto_AvailableConfig* availableConfig,
                             const char* updateFilesPath, char** statusMessage)
{
    //start normal update process
    rpmanager_Status status = runAction(availableConfig, updateFilesPath, RPMANAGER_ACTION_NORMAL_UPDATE );
    setStatusMessage(statusMessage);
    return status;
}

rpmanager_Status rpmanager_resume(const proto_AvailableConfig* availableConfig,
                             const char* updateFilesPath, char** statusMessage)
{
    //resume update process
    rpmanager_Status status = runAction(availableConfig, updateFilesPath, RPMANAGER_ACTION_RESUME_UPDATE );
    setStatusMessage(statusMessage);
    return status;
}

int rpmanager_init()
{
    LOGM(LOGG_DEBUG, "Initializing RPManager");
    if (_configStorage)
    {
        LOGM(LOGG_ERROR, "Doubled RPManager initialization!");
        return 1;
    }

    // basic setup
    _configStorage = config_newStorage(RPMANAGER_STORAGE_ID);
    if (!_configStorage)
    {
        return 1;
    }

    return 0;
}

void rpmanager_cleanup()
{
    if (_configStorage)
    {
        free(_lastMessage);

        free(_imagehandlerMessage);

        if(_installedConfig)
        {
            car_sync__proto__messages__rpcconfiguration_req__free_unpacked(_installedConfig, NULL);
            _installedConfig=NULL;
        }

        config_deleteStorage(_configStorage);
        _configStorage = 0;
    }
}
