/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "strtools.h"
#include "stdarg.h"
#include "stdlib.h"
#include "stdio.h"

extern char* strtools_sprintNew(const char* format, ...) {
    va_list args;
    va_start(args, format);
    // plus 1 for terminating null character
    unsigned int size = vsnprintf(0, 0, format, args) + 1;
    va_end(args);
    char* str = 0;
    if (size > 0) {
        str = malloc(size);
        if (str) {
            va_start(args, format);
            vsnprintf(str, size, format, args);
            va_end(args);
        }
    }
    return str;
}
