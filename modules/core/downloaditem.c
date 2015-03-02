/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "downloaditem.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

downloaditem_Item* downloaditem_newItemPreppend(downloaditem_Item** first) {
    downloaditem_Item* item = malloc(sizeof(downloaditem_Item));
    if (item) {
        memset(item, 0, sizeof(downloaditem_Item));

        item->next = *first;
        if (*first) {
            (*first)->prev = item;
        }
        *first = item;
    } else {
        LOGM(LOGG_ERROR, "Failed to allocate memory for download item!");
    }
    return item;
}

downloaditem_Item* downloaditem_deleteFirstItem(downloaditem_Item** item) {
    if (!(*item)) {
        return 0;
    }
    downloaditem_Item* next = 0;
    if ((*item)->next) {
        (*item)->next->prev = (*item)->prev;
        next = (*item)->next;
    }
    if ((*item)->prev) {
        (*item)->prev->next = (*item)->next;
        if (!next) {
            next = (*item)->prev;
        }
    }
    free((*item)->id);
    free((*item)->path);
    free((*item));
    (*item) = next;
    return next;
}

downloaditem_Item* downloaditem_findItemById(downloaditem_Item* first, const char* id) {
    while (first) {
        if (first->id && strcmp(first->id, id) == 0) {
            return first;
        }
        first = first->next;
    }
    return first;
}
