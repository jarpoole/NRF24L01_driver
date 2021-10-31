

// clang-format off
#ifndef TEST_LOGGER_H
#define TEST_LOGGER_H

#include <string.h>
#include <time.h>

/** Gets the current date/time and stores in a buffer on the stack
 * 
 *  @return The buffer with the date time
 */
static inline char* timenow() {
    static char buffer[64];
    time_t      rawtime;
    struct tm*  timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);

    return buffer;
}


// Gets the name of the file (ignores absolute path information if there is any)
#define FILENAME                   strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#define NEWLINE                    "\n"
#define LOG_FORMAT                 "%s | %-7s | %-15s | %s:%d | "
#define LOG_ARGS(LOG_TAG)          timenow(), LOG_TAG, FILENAME, __FUNCTION__, __LINE__
#define PRINTFUNCTION(format, ...) printf(format, __VA_ARGS__)

#define NO_LOG            0x00
#ifndef LOG_LEVEL
    #define LOG_LEVEL 5
#endif

/// Define a log level called 'ERROR' which prints in red
#define ERROR_LEVEL       0x01
#define ERROR_TAG         "ERROR"
#if LOG_LEVEL >= ERROR_LEVEL
    #define LOG_ERROR(message, args...) PRINTFUNCTION("\033[31m" LOG_FORMAT message "\033[0m" NEWLINE, LOG_ARGS(ERROR_TAG), ##args)
#else
    #define LOG_ERROR(message, args...)
#endif

/// Define a log level called 'INFO' which prints in green
#define INFO_LEVEL 0x02
#define INFO_TAG   "INFO"
#if LOG_LEVEL >= INFO_LEVEL
    #define LOG_INFO(message, args...) PRINTFUNCTION("\033[32m" LOG_FORMAT message "\033[0m" NEWLINE, LOG_ARGS(INFO_TAG), ##args)
#else
    #define LOG_INFO(message, args...)
#endif

/// Define a log level called 'INFO' which prints in green
#define WARNING_LEVEL 0x03
#define WARNING_TAG   "WARNING"
#if LOG_LEVEL >= WARNING_LEVEL
    #define LOG_WARNING(message, args...) PRINTFUNCTION("\033[33m" LOG_FORMAT message "\033[0m" NEWLINE, LOG_ARGS(WARNING_TAG), ##args)
#else
    #define LOG_WARNING(message, args...)
#endif

/// Define a log level called 'DEBUG' which prints in white
#define DEBUG_LEVEL 0x04
#define DEBUG_TAG   "DEBUG"
#if LOG_LEVEL >= DEBUG_LEVEL
    #define LOG_DEBUG(message, args...) PRINTFUNCTION(LOG_FORMAT message NEWLINE, LOG_ARGS(DEBUG_TAG), ##args)
#else
    #define LOG_DEBUG(message, args...)
#endif

/** Define a log level called 'VERBOSE' which prints in white
 * 
 *  This log level prints abosutely no formatting at all. 
 *  Newlines, color and formatting should be provided by the caller
 */ 
#define VERBOSE_LEVEL 0x05
#if LOG_LEVEL >= VERBOSE_LEVEL
    #define LOG_VERBOSE(message, args...) printf(message, ##args)
#else
    #define LOG_VERBOSE(message, args...)
#endif

#endif // TEST_LOGGER_H
// clang-format on