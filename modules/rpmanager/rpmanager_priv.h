/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#ifndef RPMANAGER_PRIV_H
#define RPMANAGER_PRIV_H

#include "rpmanager.h"

# ifdef __cplusplus
extern "C" {
# endif

typedef enum {
    RPMANAGER_STATE_UPDATE_SUCCESS = 0, /** Update was successful. */
    RPMANAGER_STATE_ROLLBACK_SUCCESS, /** Update was not successful, but rollback succeeded. */
    RPMANAGER_STATE_UPDATE_PENDING, /** Update failed because it was interrupted (for example due to power off). */
    RPMANAGER_STATE_ROLLBACK_PENDING /** Rollback failed because it was interrupted (for example due to power off) or update failed because of error and rollback is needed. */
} rpmanager_State;

static const char* RPMANAGER_STORAGE_ID = "rpmanager";
static const char* INSTALLED_CONFIG_KEY = "installed_config";
static const char* RPMANAGER_STATE_KEY = "rpmanager_state";
static const char* LAST_INSTALLED_UF_NB_KEY = "last_installed_uf_nb";

# ifdef __cplusplus
}
# endif

#endif
