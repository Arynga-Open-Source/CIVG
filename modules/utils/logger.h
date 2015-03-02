/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * CIVG logger.
 */

#ifndef LOGGER_H
#define LOGGER_H

# ifdef __cplusplus
extern "C" {
# endif

#ifndef LOGGER_MODULE
#define LOGGER_MODULE "CIVG"
#endif

#ifndef LOGGER_FILE
#define LOGGER_FILE "civg.log"
#endif

typedef enum{
    LOGG_ERROR = 1, /** Critical app error e.g. repository conf access error (quite mode). */
    LOGG_WARNING, /** Warning about failed action e.g. invalid message received (normal/default mode). */
    LOGG_INFO, /** Important info about app flow e.g. app startup, update found (verbose mode). */
    LOGG_DEBUG /**  Detailed info about app flow for debug puroses e.g. LOG_FUNCTION. */
} logger_LogLevel;

/**
 * @brief Sets logger verbosity.
 * @param[in] level The highest log level that will be printed out.
 */
extern void logger_setLevel(logger_LogLevel level);

/**
 * @brief Write formated log message to specifed file and stdlog/stderr.
 * @param[in] file Name of log file (without path).
 * @param[in] module Name of module (added to log prefix)
 * @param[in] level Log level (added to log prefix).
 * @param[in] format Log message format.
 */
extern void logger_log(const char* fileName, const char* module, logger_LogLevel level, const char* format, ...);

#define LOG(level, format, ...) \
    logger_log(LOGGER_FILE, LOGGER_MODULE, level, format, __VA_ARGS__)

#define LOGM(level, msg) \
    logger_log(LOGGER_FILE, LOGGER_MODULE, level, msg)

#ifdef _DEBUG
    #include <assert.h>
    #define TRACE(format, ...) \
        logger_log(LOGGER_FILE, LOGGER_MODULE, LOGG_DEBUG, format, __VA_ARGS__)
#define TRACEM(msg) \
    logger_log(LOGGER_FILE, LOGGER_MODULE, LOGG_DEBUG, msg)
    #define ASSERT(x) assert(x)
#else
    #define TRACE(format, ...)
    #define TRACEM(msg)
    #define ASSERT(x)
#endif

# ifdef __cplusplus
}
# endif

#endif
