/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * This is library of functions that gets vehicle specific information like VIN/speed
 * or system/hardware configuration.
 */

#ifndef CARINFO_H
#define CARINFO_H

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @brief Gets VIN (Vehicle Identification Number).
 * @param[out] vin 0 terminated string with VIN or NULL in error case.
 *       Returned string is dynamically alocated and must be freed when
 *       no more needed.
 * @return 0 if succeed, 1 otherwise.
 */
extern int carinfo_getVin(char** vin);

# ifdef __cplusplus
}
# endif

#endif
