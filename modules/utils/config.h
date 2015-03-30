/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * OS specific implementation of config storage.
 */

#ifndef CONFIG_H
#define CONFIG_H

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @brief Opaque structure that identify specific config storage.
 */
typedef struct config_Storage config_Storage;

/**
 * @brief Creates new storage structure for given (unique) id.
 * @param[in] id Unique id that identify config storage.
 * @return Pointer to opaque storage strucuture.
 */
extern config_Storage* config_newStorage(const char* id);

/**
 * @brief Releases all resources related to storage.
 * @param[in] storage Pointer to storage structure.
 */
extern void config_deleteStorage(config_Storage* storage);

/**
 * @brief Sets byte array value for given key in specifed storage.
 * @param[in] storage Storage pointer.
 * @param[in] key Key of element to be set.
 * @param[in] value Value to be set.
 * @param[in] size Size of value array (in bytes).
 * @return 0 if succeed, 1 otherwise.
 */
extern int config_setValue(config_Storage* storage, const char* key, const void* value, int size);

/**
 * @brief Gets byte array value of element with given key in specifed storage.
 * @param[in] storage Storage pointer.
 * @param[in] key Key of element.
 * @param[out] value Pointer to newly allocated byte array with element value.
 *       Caller of this function is responsible for releasing allocated
 *       memory when it is not needed anymore.
 * @param[out] size Size of returned value array (in bytes).
 * @return 0 if succeed, 1 otherwise.
 */
extern int config_getValue(config_Storage* storage, const char* key, void** value, int *size);

/**
 * @brief Sets int value for given key in specifed storage.
 * @param[in] storage Storage pointer.
 * @param[in] key Key of element to be set.
 * @param[in] value Value to be set.
 * @return 0 if succeed, 1 otherwise.
 */
extern int config_setIValue(config_Storage* storage, const char* key, int value);

/**
 * @brief Gets int value of element with given key in specifed storage.
 * @param[in] storage Storage pointer.
 * @param[in] key Key of element.
 * @param[out] value Element value.
 * @return 0 if succeed, 1 otherwise.
 */
extern int config_getIValue(config_Storage* storage, const char* key, int* value);

/**
 * @brief Removes key and associated value from specified storage.
 * @param[in] storage Storage pointer.
 * @param[in] key Key of element to be removed.
 * @return 0 if succeed, 1 otherwise.
 */
extern int config_removeKey(config_Storage* storage, const char* key);

# ifdef __cplusplus
}
# endif

#endif
