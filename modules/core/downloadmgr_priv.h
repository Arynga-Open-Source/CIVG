/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#ifndef DOWNLOADMGR_PRIV_H
#define DOWNLOADMGR_PRIV_H

#include "downloadmgr.h"

static const char* DOWNLOADMGR_STORAGE_ID = "civg_downloadmgr";
static const char* DI_SIZE_KEY = "downloaded_items_size";
static const char* DI_PATH_KEY_FORMAT = "item_%d_path";
#define ITEM_KEY_SIZE 21
static char itemKey[ITEM_KEY_SIZE] = "\0";

#endif
