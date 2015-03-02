/* Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */
#include <errno.h>
#include <stdio.h>
#include <cstdlib>

#include <carsync-hu-south-proto.h>
#include <carsync-hu-south-proto-utils.h>

typedef CarSync__Proto__ReleasePackage__UpdateFileMeta UpdateFileMeta;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
#ifdef QNX_BUILD
        fprintf(stderr, "usage: %s  <path-to-uf>\n", argv[0]);
#else
        fprintf(stderr, "usage: %s  <path-to-uf>\n", program_invocation_name);
#endif
        return 1;
    }
	
	size_t size = 0; 
    const char* data = cs_proto_get_uf_meta(argv[1], &size);

	FILE* f = NULL;
	if( (f=fopen("data.uf", "wb")) == NULL)
	{
		fprintf(stderr, "Cannot write extracted data\n");
		return 1;
	}
	if( fwrite(data, sizeof(char), size, f) != size)
	{
		fprintf(stderr, "error while writing data \n");
		fclose(f);
		return 2;
	}
	fclose(f);	

    UpdateFileMeta* uf;

	
	// Unpack the message using protobuf-c.
    uf = car_sync__proto__release_package__update_file_meta__unpack(NULL, size, (uint8_t*)data);
	if (uf == NULL)
	{
	  fprintf(stderr, "error unpacking protobuf message\n");
	  return 3;
	}


    printf("Update file: %s \n", uf->uuid);
    printf("URL: %s \n", uf->url);
	printf("version: %d.%d.%d\n", uf->version->major_version, uf->version->minor_version, uf->version->build_version); 
	printf("checksum %s \n", uf->checksum);

    free((char*)data);
    car_sync__proto__release_package__update_file_meta__free_unpacked(uf, NULL);
    return 0;
}
