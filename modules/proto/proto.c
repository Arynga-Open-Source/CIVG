/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "proto.h"
#include "filetools.h"
#include <string.h>
#include <stdlib.h>

static const char* META_FILENAME = "UF.meta";

extern int proto_versionCmp(const proto_Version *a, const proto_Version *b)
{
    if  (!a) {
        return (!b) ? 0 : -1;
    } else  if (!b) {
        return 1;
    }

    if (!a->has_major_version) {
        return (!b->has_major_version) ? 0 : -1;
    } else if (!b->has_major_version) {
        return 1;
    }
    if (a->major_version == b->major_version) {
        if (!a->has_minor_version) {
            return (!b->has_minor_version) ? 0 : -1;
        } else if (!b->has_minor_version) {
            return 1;
        }
        if (a->minor_version == b->minor_version) {
            if (!a->has_build_version) {
                return (!b->has_build_version) ? 0 : -1;
            } else if (!b->has_build_version) {
                return 1;
            }
            if (a->build_version == b->build_version) {
                return 0;
            } else {
                return (a->build_version > b->build_version) ? 1 : -1;
            }
        } else {
            return (a->minor_version > b->minor_version) ? 1 : -1;
        }
    } else {
        return (a->major_version > b->major_version) ? 1 : -1;
    }}


proto_UpdateFileMeta *proto_unpackMetaFile(const char *ufMetaDir)
{
    proto_UpdateFileMeta* ufMeta = 0;
    if (ufMetaDir && strlen(ufMetaDir)) {
        char* metaPath = filetools_joinPath(ufMetaDir, META_FILENAME);
        if (metaPath) {
            void* data;
            size_t dataSize;
            if (filetools_readAll(metaPath, &data, &dataSize) == 0) {
                ufMeta = car_sync__proto__release_package__update_file_meta__unpack(NULL,dataSize, data);
                free(data);
            }
            free(metaPath);
        }
    }
    return ufMeta;
}
