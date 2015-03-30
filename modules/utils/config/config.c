/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <pthread.h>
#include "logger.h"
#include "strtools.h"
#include "filetools.h"

#define DB_TABLE "storage"
#define CREATE_TBL  \
    "CREATE TABLE IF NOT EXISTS " DB_TABLE " ("                     \
    " \"key\" TEXT UNIQUE NOT NULL,"                             \
    " \"data\" TEXT "                                            \
    ");"

struct config_Storage {
    sqlite3* _db;            ///< db handle
    char* path;             ///< path to the database
    sqlite3_stmt* stmt;     ///< sqlite statement
    pthread_mutex_t _lock;  /* sqlite lock */
};

static int _handlesCount = 0;

/**
 * @brief prepare DB statement
 *
 * DB is locked
 */
int dbPrepare(config_Storage* storage, const char* query)
{
    if(!query || strlen(query) == 0) {
        return 1;
    }

    return sqlite3_prepare(storage->_db, query, strlen(query),
                           &storage->stmt, NULL) != SQLITE_OK;
}

/**
 * @brief iterate over results
 *
 * DB is locked
 */
int dbStep(config_Storage* storage)
{
    int rc = sqlite3_step(storage->stmt);
    return (rc != SQLITE_ROW && rc != SQLITE_DONE);
}

/**
 * @brief finalize DB statement
 *
 * DB is locked
 */
void dbFinalize(config_Storage* storage)
{
    if(storage->stmt){
        sqlite3_finalize(storage->stmt);
        storage->stmt = 0;
    }
}

/**
 * @brief simple DB query
 *
 * DB is locked
 */
static int dbSimple(config_Storage* storage, const char* query)
{
    if(!query || strlen(query) == 0) {
        return 1;
    }
    int status = 1;

    if(dbPrepare(storage, query) == 0) {
        status = dbStep(storage);
    }

    dbFinalize(storage);

    return status;
}

/**
 * @brief read column value as integer
 *
 * DB is locked
 */
int dbColAsInt(config_Storage* storage, int col, int* out)
{
    if(!storage->stmt
       || col < 0
       || col > sqlite3_column_count(storage->stmt)) {
        return 1;
    }

    *out = sqlite3_column_int(storage->stmt, col);
    return 0;
}

/**
 * @brief read column value as blob
 *
 * DB is locked
 */
int dbColAsBlob(config_Storage* storage, int col, const void** out, int* size)
{
    if(!storage->stmt
       || col < 0
       || col > sqlite3_column_count(storage->stmt)) {
        return 1;
    }

    *out = sqlite3_column_blob(storage->stmt, col);
    *size = sqlite3_column_bytes(storage->stmt, col);
    return 0;
}

/**
 * @brief allocate new storage
 *
 * Initializes configuration wrapper, DB is openend, access mutex gets
 * initialized
 */
config_Storage* config_newStorage(const char* id) {
    if (!id || strlen(id) == 0) {
        LOGM(LOGG_ERROR, "Config storage id can not be empty!");
        return 0;
    }

    config_Storage* storage = malloc(sizeof(config_Storage));
    if (!storage) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for config storage!");
        return 0;
    }
    if(_handlesCount == 0
       && sqlite3_initialize() != SQLITE_OK) {
            free(storage);
            LOGM(LOGG_ERROR, "sqlite3_initialize failed!");
            return 0;
    }
    ++_handlesCount;
    /* init storage lock */
    pthread_mutex_init(&storage->_lock, NULL);

    storage->_db = 0;
    storage->path = filetools_joinPath(CONFIG_DIR, id);
    storage->stmt = 0;

    if(sqlite3_open_v2(storage->path, &storage->_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL ) != SQLITE_OK) {
        LOG(LOGG_ERROR, "sqlite3_open_v2 failed! Path: %s", storage->path);
        config_deleteStorage(storage);
        storage = 0;
    } else if (dbSimple(storage, CREATE_TBL) != 0) { // creates table if it doesn't exist
        LOG(LOGG_ERROR, "Failed to create table! Id: %s", id);
        config_deleteStorage(storage);
        storage = 0;
    }
    return storage;
}

/**
 * @brief destroy configuration wrapper
 *
 * NOTE: access mutex is not checked, may corrupt DB if called form
 * multiple threads
 */
void config_deleteStorage(config_Storage* storage) {
    // close connection with the database and release memory allocated in the constructor
    if(storage)
    {
        sqlite3_close(storage->_db);
        free(storage->path);
        pthread_mutex_destroy(&storage->_lock);
        free(storage);

        if(_handlesCount > 0) {
            --_handlesCount;
            if (_handlesCount == 0) {
                sqlite3_shutdown();
            }
        }
    }
}

/**
 * @brief check if key exist
 *
 * DB is locked
 */
static int exists(config_Storage* storage, const char* key)
{
    if (!key || strlen(key) == 0) {
        LOGM(LOGG_ERROR, "Can not check empty key!");
        return 0;
    }
    char* query = strtools_sprintNew("SELECT count(*) from " DB_TABLE
                                     " WHERE \"key\" = \"%s\";", key);
    if (!query) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for query string!");
        return 0;
    }
    int result = 0;
    if (dbPrepare(storage, query) == 0) {
        if (dbStep(storage) == 0) {
            dbColAsInt(storage, 0, &result);
        }
        dbFinalize(storage);
    }
    free(query);
    return result;
}

/**
 * @brief check if key exists
 *
 * DB access is locked
 */
static int keyFullCheck(config_Storage* storage, const char* key) {
    if (!key || strlen(key) == 0) {
        LOGM(LOGG_ERROR, "Empty key received!");
        return 1;
    }
    if (!exists(storage, key)) {
        LOGM(LOGG_DEBUG, "Key not found!");
        return 1;
    }
    return 0;
}

int config_setValue(config_Storage* storage, const char* key, const void* value, int size) {
    int status = 1;
    if (!key || strlen(key) == 0) {
        LOGM(LOGG_ERROR, "Empty key received!");
        return 1;
    }

    /* lock */
    pthread_mutex_lock(&storage->_lock);

    char* query = (exists(storage,key))?
                  strtools_sprintNew("UPDATE " DB_TABLE " SET \"data\"=?"
                                     " WHERE \"key\" = \"%s\";", key):
                  strtools_sprintNew("INSERT INTO " DB_TABLE " ( \"key\", \"data\" )"
                                     " VALUES ( \"%s\", ? );", key);
    if (!query) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for query string!");
        goto cleanup;
    }

    if (dbPrepare(storage, query) == 0) {
        if (sqlite3_bind_blob(storage->stmt, 1, value, size, SQLITE_STATIC) == SQLITE_OK
            && dbStep(storage) == 0) {
            status = 0;
        }
        dbFinalize(storage);
    }
    free(query);

cleanup:
    /* unlock */
    pthread_mutex_unlock(&storage->_lock);

    return status;
}

int config_getValue(config_Storage* storage, const char* key, void** value, int* size) {
    int status = 1;

    if (!storage)
        return 1;

    /* lock */
    pthread_mutex_lock(&storage->_lock);

    if (keyFullCheck(storage, key) != 0) {
        goto cleanup;
    }
    char* query = strtools_sprintNew("SELECT data FROM " DB_TABLE
                                     " WHERE \"key\" = \"%s\";", key);
    if (!query) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for query string!");
        goto cleanup;
    }

    if (dbPrepare(storage, query) == 0) {
        if (dbStep(storage) == 0) {
            const void* buffer;
            if (dbColAsBlob(storage, 0, &buffer, size) == 0) {
                if (*size > 0) {
                    *value = malloc(*size);
                    if (buffer) {
                        memcpy(*value, buffer, *size);
                        status = 0;
                    } else {
                        LOGM(LOGG_ERROR, "Failed to allocate memory for value!");
                    }
                } else {
                    *value = 0;
                    status = 0;
                }
            }
        }
        dbFinalize(storage);
    }
    free(query);

cleanup:
    /* unlock */
    pthread_mutex_unlock(&storage->_lock);

    return status;
}

int config_setIValue(config_Storage* storage, const char* key, int value) {
    int status = 1;

    if (!key || strlen(key) == 0) {
        LOGM(LOGG_ERROR, "Empty key received!");
        return 1;
    }

    if (!storage)
        return 1;

    /* lock */
    pthread_mutex_lock(&storage->_lock);

    char* query = (exists(storage,key))?
                  strtools_sprintNew("UPDATE " DB_TABLE " SET \"data\"=\"%d\""
                                     " WHERE \"key\" = \"%s\";", value, key):
                  strtools_sprintNew("INSERT INTO " DB_TABLE " ( \"key\", \"data\" )"
                                     " VALUES ( \"%s\", \"%d\" );", key, value);
    if (!query) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for query string!");
        goto cleanup;
    }

    status = dbSimple(storage, query);
    free(query);

cleanup:
    /* unlock */
    pthread_mutex_unlock(&storage->_lock);

    return status;
}

int config_getIValue(config_Storage* storage, const char* key, int* value){
    int status = 1;

    if (!storage)
        return 1;

    /* lock */
    pthread_mutex_lock(&storage->_lock);

    if (keyFullCheck(storage, key) != 0) {
        goto cleanup;
    }
    char* query = strtools_sprintNew("SELECT data FROM " DB_TABLE
                                     " WHERE \"key\" = \"%s\";", key);
    if (!query) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for query string!");
        goto cleanup;
    }
    if (dbPrepare(storage, query) == 0) {
        if (dbStep(storage) == 0) {
            status = dbColAsInt(storage, 0, value);
        }
        dbFinalize(storage);
    }
    free(query);

cleanup:
    /* unlock */
    pthread_mutex_unlock(&storage->_lock);

    return status;
}

int config_removeKey(config_Storage* storage, const char* key) {
    int status = 1;

    if (!storage)
        return 1;

    /* lock */
    pthread_mutex_lock(&storage->_lock);

    if (keyFullCheck(storage, key) != 0) {
        goto cleanup;
    }
    char* query = strtools_sprintNew("DELETE FROM " DB_TABLE
                                     " WHERE \"key\" = \"%s\";", key);
    if (!query) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for query string!");
        goto cleanup;
    }
    status = dbSimple(storage, query);
    free(query);

cleanup:
    /* unlock */
    pthread_mutex_unlock(&storage->_lock);

    return status;
}
