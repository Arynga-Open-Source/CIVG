#ifndef _CS_SP_UTILS_H_
#define _CS_SP_UTILS_H_

#include <string>

namespace CarSync {
    namespace Proto {
        namespace Utils
        {
            /**
             * @brief return contents of UF metadata from given UF file
             */
            std::string get_uf_meta(const std::string &uf_path);
        }
    }
}

#endif /* _CS_SP_UTILS_H_ */
