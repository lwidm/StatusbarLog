// -- StatusbarLog/include/StatusbarLog.h

#ifndef STATUSBARLOG_H
#define STATUSBARLOG_H

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

/**
 * \enum Log_level
 * \brief Defines the different log levels for logging messages that can be used
 * to catagorize the importance of the message being logged, or ignore messages
 * of lower importance than the minim log level specified.
 *
 * - \c LOG_LEVEL_OFF: No logging
 * - \c LOG_LEVEL_ERR: Logs errors to the console with the prefix "ERROR" (using
 * the `print_err` <- \todo function)
 * - \c LOG_LEVEL_WRN: Logs warnings to the console with the prefix "WARNING".
 * - \c LOG_LEVEL_INF: Logs informational messages to the console with the
 * prefix "INFO".
 * - \c LOG_LEVEL_DBG: Logs debug messages to the console with the prefix
 * "DEBUG".
 *
 *\see StatusbarLog::log: Logginf function -> \todo
 *\see StatusbarLog::log_LEVEL: Globally set log level
 * \see print_err: Function for printing error messages -> \todo
 */
// clang-format off
typedef enum {
  LOG_LEVEL_OFF = 0, ///< No logging
  LOG_LEVEL_ERR, ///< Logs errors to the console with the prefix "ERROR" (using the `print_err` <- \todo function)
  LOG_LEVEL_WRN, ///< Logs warnings to the console with the prefix "WARNING".
  LOG_LEVEL_INF, ///< Logs informational messages to the console with the prefix "INFO".
  LOG_LEVEL_DBG, ///< Logs debug messages to the console with the prefix "DEBUG".
} Log_level;
// clang-format on

/**
 * \def LOG_LEVEL
 * \brief Deines the global log level for the logging system
 *
 * This macro defines the global log level for the logging system. Messages with
 * a log level can be adjusted to control the verbosity of logging output.
 *
 * \see StatusbarLog::log: Logging function
 *\see StatusbarLog::log_level: Different kinds of log levels
 */
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DBG
#endif  // !LOG_LEVEL

namespace StatusbarLog {

typedef struct {
  std::vector<unsigned int> positions;
  std::vector<unsigned int> bar_sizes;
  std::vector<std::string> prefixes;
  std::vector<std::string> postfixes;
  std::vector<std::size_t> spin_idxs;
} ProgressBar;

extern std::vector<ProgressBar> progressbars;


/**
 * \brief Logs a formatted message to the console if its level is
 *        at or below the global \c LOG_LEVEL.
 *
 * \param filename  Source filename or tag (will be printed in log message,
 * should be the origin of the log message).
 * \param fmt       printf-style format string.
 * \param log_level Severity level of this message.
 * \param ...       Additional format arguments.
 *
 * \return 0 on success, non-zero on formatting error.
 *
 *\see StatusbarLog::log_level: Different kinds of log levels
 *\see StatusbarLog::log_LEVEL: Globally set log level
 * \see print_err: Function for printing error messages -> \todo
 */
int log(const std::string& filename, const std::string& fmt,
        Log_level log_level, ...);

/**
 * \def LOG_ERR
 * \brief Shortcut for logging warnings.
 *
 *\see StatusbarLog::log: General logging function
 */
#define LOG_ERR(filename, fmt, ...) \
  StatusbarLog::log(filename, fmt, LOG_LEVEL_ERR, ##__VA_ARGS__)

/**
 * \def LOG_WRN
 * \brief Shortcut for logging warnings.
 *
 *\see StatusbarLog::log: General logging function
 */
#define LOG_WRN(filename, fmt, ...) \
  StatusbarLog::log(filename, fmt, LOG_LEVEL_WRN, ##__VA_ARGS__)

/**
 * \def LOG_INF
 * \brief Shortcut for logging informational messages.
 *
 *\see StatusbarLog::log: General logging function
 */
#define LOG_INF(filename, fmt, ...) \
  StatusbarLog::log(filename, fmt, LOG_LEVEL_INF, ##__VA_ARGS__)

/**
 * \def LOG_DBG
 * \brief Shortcut for logging debug messages.
 *
 *\see StatusbarLog::log: General logging function
 */
#define LOG_DBG(filename, fmt, ...) \
  StatusbarLog::log(filename, fmt, LOG_LEVEL_DBG, ##__VA_ARGS__)

int create_progressbar(ProgressBar& progressbar,
                       const std::vector<unsigned int> _positions,
                       const std::vector<unsigned int> _bar_sizes,
                       const std::vector<std::string> _prefixes,
                       const std::vector<std::string> _postfixes);

int update_progress_bar(ProgressBar& progressbar, const std::size_t idx,
                        const double percent);

}  // namespace StatusbarLog

#endif  // !STATUSBARLOG_H
