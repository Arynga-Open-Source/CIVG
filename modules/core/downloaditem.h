/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * This library generates list of files to be downloaded (based on update meta)
 * and passes it to actual downloader.
 */

#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

typedef enum {
    DOWNLOADITEM_TYPE_META = 0,
    DOWNLOADITEM_TYPE_DATA
} downloaditem_ItemType;

struct downloaditem_Item {
    char* id;
    char* path;
    downloaditem_ItemType type;
    struct downloaditem_Item* next;
    struct downloaditem_Item* prev;
    unsigned int progress;
};
typedef struct downloaditem_Item downloaditem_Item;

downloaditem_Item* downloaditem_newItemPreppend(downloaditem_Item** first);

downloaditem_Item* downloaditem_deleteFirstItem(downloaditem_Item **item);

downloaditem_Item* downloaditem_findItemById(downloaditem_Item* first, const char* id);

#endif
