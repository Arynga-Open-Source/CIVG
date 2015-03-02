#include "carsync-hu-south-proto-utils.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <errno.h>
#include <tar.h>
#define LOGGER_NAME     "proto-utils"
#define LOGGER_COL      "33m"

#include <carsync/carsync-hu-common-logger.h>


/** taken from tar(5) */
struct header_posix_ustar {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];              // contains TMAGIC
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

std::string
CarSync::Proto::Utils::get_uf_meta(const std::string &uf_path)
{
    logger(LOGG_DEBUG, "extract UF meta from %s\n", uf_path.c_str());

    std::ifstream tar(uf_path, std::ios::binary);
    if (tar.bad() || !tar.is_open())
    {
        logger(LOGG_ERROR, "error: failed to open tar file: %s\n",
               strerror(errno));
    }

#define TAR_BLOCK_SIZE 512

    // end of tar is identified by double empty (all 0) blocks
    char zero_block[TAR_BLOCK_SIZE];
    memset(zero_block, 0, sizeof(zero_block));

    int end_cnt = 0;            // zeroed blocks count - 2 blocks
                                // indicate archive end
    int in_meta_file = 0;       // inside of UF metadata file
    int in_file = 0;            // inside of file data
    long long in_file_size = 0; // file size
    long long cur_size = 0;     // read so far size

    std::stringstream meta;

    while (!tar.eof() && !tar.bad())
    {
        char block[TAR_BLOCK_SIZE];
        size_t rd = 0;

        do
        {
            size_t rd_now = tar.readsome(block + rd,
                                         sizeof(block) - rd);

           if (!tar)
                 break;
           if (rd_now == 0 && errno != EINTR)
                 break;

           rd += rd_now;

        } while (rd < TAR_BLOCK_SIZE);

        if (rd != TAR_BLOCK_SIZE)
        {
            logger(LOGG_DEBUG, "end of stream\n");
            break;
        }

        // we're inside a file
        if (in_file)
        {
            if (in_meta_file)
            {
                // check how much data to write to the stream
                long long to_write = in_file_size - cur_size;
                if (to_write > TAR_BLOCK_SIZE)
                    to_write = TAR_BLOCK_SIZE;

                meta.write(block, to_write);
            }

            cur_size += TAR_BLOCK_SIZE;
            if (cur_size >= in_file_size)
            {
                // file complete
                in_file = 0;
                in_file_size = 0;
                cur_size = 0;
            }

            // inside of meta file, but file size check indicates that
            // file is complete, thus meta file is complete, time to
            // return
            if (in_meta_file && in_file == 0)
            {
                logger(LOGG_DEBUG, "meta file complete\n");
                break;
            }
            continue;
        }

        // check if it's the end of the archive
        if (memcmp(block, zero_block, sizeof(block)) == 0)
        {
            end_cnt++;
            if (end_cnt == 2)
                break;
            else
                continue;
        }
        else
            end_cnt = 0;

        // not an end, so perhaps a header
        header_posix_ustar *hdr = reinterpret_cast<header_posix_ustar*>(block);

        // verify basic fields - version and magic
        if (memcmp(hdr->magic, TMAGIC, TMAGLEN) != 0 && // POSIX tar
            memcmp(hdr->magic, "ustar ", strlen("ustar ")) != 0) // GNU tar
            {
                logger(LOGG_ERROR, "error: unknown tar file format, magic: \'%s\'\n",
                       hdr->magic);
                continue;
            }

        // check if we're looking at regular file, skip other files
        if (hdr->typeflag[0] == REGTYPE || hdr->typeflag[0] == AREGTYPE)
        {
            char *pos = hdr->name;
            char *err = hdr->size;
            long long size = strtoull(hdr->size, &err, 8);

            if (*err != '\0' || size == 0)
            {
                logger(LOGG_ERROR, "error: invalid size");
                continue;
            }
            in_file_size = size;
            in_file = 1;

            // check if it's a UF.meta

            // skip leading ./ if present
            if (pos[0] == '.' && pos[1] == '/')
                pos +=2;

            if (strncmp(pos, "UF.meta", sizeof("UF.meta")) == 0)
            {
                // find length of meta file
                logger(LOGG_DEBUG, "found UF.meta, %lld bytes of data follows\n",
                    in_file_size);

                in_meta_file = 1;
            }
        }

    }

    return meta.str();
}
