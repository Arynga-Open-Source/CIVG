/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * Compiler specific definitions and macros.
 */

#ifndef __CIVG_COMPILER_H__
#define __CIVG_COMPILER_H__

/* covers GCC and clang */
#if defined(__GNUC__) || defined(__clang__)
#define CIVG_UNUSED __attribute__((unused))
#else
#define CIVG_UNUSED
#endif

#endif /* __CIVG_COMPILER_H__ */
