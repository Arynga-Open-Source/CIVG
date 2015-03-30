/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "dummy_carinfo_priv.h"
#include "config.h"

int carinfo_getVin(char** vin)
{
    int status = 1;
    config_Storage* storage = config_newStorage(DUMMY_CARINFO_STORAGE_ID);
    if (storage) {
        int size;
        if (config_getValue(storage, VIN_KEY, (void**)vin, &size) == 0) {
            status = 0;
        }
        config_deleteStorage(storage);
    }
    return status;
}
