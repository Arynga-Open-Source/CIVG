/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "core.h"
#include "filetools.h"
#include "build_version.h"
#include "config.h"
#include "dummy_carinfo_priv.h"
#include "rvi_config_priv.h"
#include "rpmanager_priv.h"
#include "core_priv.h"
#include "proto.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>

// Options
static int printVersion = 0;
static const char* configPath = 0;

static void printUsage(int argc, char** argv) {
    const char* appName = "civg_config";
    if (argc > 0) {
        char* fName = strrchr(argv[0], '/');
        if (fName && strlen(++fName) != 0) {
            appName = fName;
        }
    }
    printf("Usage:\n"
           "  %s [OPTION...] CONFIG_JSON_PATH - CarSync In Vehicle Gateway config tool\n\n"
           "Help Options:\n"
           "  -h, --help             Show help options\n\n"
           "Application Options:\n"
           "  -v, --version          Print version and exit.\n\n",
           appName);
}

static int parseOpts(int argc, char** argv)
{
    int status = 0;
    if (argc == 2) {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            status = 1;
            printVersion = 1;
        } else if (strlen(argv[1]) > 0 && argv[1][0] != '-') {
            status = 1;
            configPath = argv[1];
        }
    }
    return status;
}

static void applyRPManagerConfig(cJSON* root) {
    (void)LAST_INSTALLED_UF_NB_KEY;
    config_Storage* conf = config_newStorage(RPMANAGER_STORAGE_ID);
    if (conf) {
        config_setIValue(conf, RPMANAGER_STATE_KEY, RPMANAGER_STATE_UPDATE_SUCCESS);

        proto_InstalledConfig instConfProto = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_REQ__INIT;
        cJSON* installedConfig = cJSON_GetObjectItem(root, "installedConfig");
        if (installedConfig && installedConfig->type == cJSON_Array) {
            instConfProto.n_rps = cJSON_GetArraySize(installedConfig);
            instConfProto.rps = malloc(instConfProto.n_rps * sizeof(proto_ReleasePackageMeta*));
            if (instConfProto.rps) {
                cJSON *rp, *uuid, *version, *major, *minor, *build;
                for (size_t i = 0; i < instConfProto.n_rps; ++i) {
                    instConfProto.rps[i] = malloc(sizeof(proto_ReleasePackageMeta));
                    car_sync__proto__release_package__release_package_meta__init(instConfProto.rps[i]);

                    rp = cJSON_GetArrayItem(installedConfig, i);
                    if (rp && rp->type == cJSON_Object) {
                        uuid = cJSON_GetObjectItem(rp, "uuid");
                        if (uuid && uuid->type == cJSON_String) {
                            instConfProto.rps[i]->uuid = uuid->valuestring;
                        }
                        version = cJSON_GetObjectItem(rp, "version");
                        if (version && version->type == cJSON_Object) {
                            instConfProto.rps[i]->version = (proto_Version*)malloc(sizeof(proto_Version));
                            car_sync__proto__common__version__init(instConfProto.rps[i]->version);

                            major = cJSON_GetObjectItem(version, "major");
                            if (major && major->type == cJSON_Number) {
                                instConfProto.rps[i]->version->has_major_version = 1;
                                instConfProto.rps[i]->version->major_version = major->valueint;
                            }
                            minor = cJSON_GetObjectItem(version, "minor");
                            if (minor && minor->type == cJSON_Number) {
                                instConfProto.rps[i]->version->has_minor_version = 1;
                                instConfProto.rps[i]->version->minor_version = minor->valueint;
                            }
                            build = cJSON_GetObjectItem(version, "build");
                            if (build && build->type == cJSON_Number) {
                                instConfProto.rps[i]->version->has_build_version = 1;
                                instConfProto.rps[i]->version->build_version = build->valueint;
                            }
                        }
                    }
                }
            }
        }

        int bufferSize = car_sync__proto__messages__rpcconfiguration_req__get_packed_size(&instConfProto);
        void* buffer = malloc(bufferSize);
        if (buffer) {
            car_sync__proto__messages__rpcconfiguration_req__pack(&instConfProto, buffer);
            config_setValue(conf, INSTALLED_CONFIG_KEY, buffer, bufferSize);
            free(buffer);
        }

        if (instConfProto.rps) {
            for (size_t i = 0; i < instConfProto.n_rps; ++i) {
                free(instConfProto.rps[i]->version);
                free(instConfProto.rps[i]);
            }
            free(instConfProto.rps);
        }

        config_deleteStorage(conf);
    }
}

static void applyRVIConfig(cJSON* root) {
    config_Storage* conf = config_newStorage(RVI_STORAGE_ID);
    if (conf) {
        cJSON* user = cJSON_GetObjectItem(root, "rvi_node");
        if (user && user->type == cJSON_String) {
            config_setValue(conf, RVI_NODE_ADDR_KEY, user->valuestring, strlen(user->valuestring) + 1);
        }
        config_deleteStorage(conf);
    }
}

static void applyBaseConfig(cJSON* root) {
    config_Storage* conf = config_newStorage(DUMMY_CARINFO_STORAGE_ID);
    if (conf) {
        cJSON* vin = cJSON_GetObjectItem(root, "VIN");
        if (vin && vin->type == cJSON_String) {
            config_setValue(conf, VIN_KEY, vin->valuestring, strlen(vin->valuestring) + 1);
        }
        config_deleteStorage(conf);
    }
    (void)AVAILABLE_CONFIG_KEY;
    conf = config_newStorage(CORE_STORAGE_ID);
    if (conf) {
        config_setIValue(conf, STATE_KEY, 0);
        config_deleteStorage(conf);
    }
}

int main(int argc, char *argv[])
{
    if (!parseOpts(argc, argv)) {
        printUsage(argc, argv);
        return 1;
    }

    if (printVersion) {
        printf("CarSync In Vehicle Gateway config tool 2.0.%s.\n", BUILD_VERSION);
        return 0;
    }

    if (configPath) {
        char* fContent;
        size_t fSize;
        if (filetools_readAll(configPath, (void**)&fContent, &fSize) == 0) {
            cJSON* root = cJSON_Parse(fContent);
            if (root) {
                applyBaseConfig(root);
                applyRVIConfig(root);
                applyRPManagerConfig(root);

                cJSON_Delete(root);
            } else {
                printf("Config/JSON syntax error before: [%s]\n",cJSON_GetErrorPtr());
            }
            free(fContent);
        } else {
            printf("Could not read content of file: %s\n", configPath);
        }
    }

    return 0;
}
