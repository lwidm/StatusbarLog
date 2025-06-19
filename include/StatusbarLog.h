// -- StatusbarLog/include/StatusbarLog.h

#ifndef STATUSBARLOG_H
#define STATUSBARLOG_H

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

/**
 * \enum Log_level
 * \brief Defines log levels for categorizing message importance.
 *
 * Messages below the minimum log level (LOG_LEVEL) are ignored.
 *
 * Levels:
 * - \c LOG_LEVEL_OFF: No logging
 * - \c LOG_LEVEL_ERR: Logs errors to the console with the prefix "ERROR" (using
 *the `print_err` <- \todo function)
 * - \c LOG_LEVEL_WRN: Logs warnings to the console with the prefix "WARNING".
 * - \c LOG_LEVEL_INF: Logs informational messages to the console with the
 *prefix "INFO".
 * - \c LOG_LEVEL_DBG: Logs debug messages to the console with the prefix
 *"DEBUG".
 *
 *\see StatusbarLog::log: Actual function used for creating log messages.
 *\see StatusbarLog::LOG_LEVEL: Macro to set the global logging threshold.
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
 * \brief Sets the global logging verbosity threshold.
 *
 * Only messages with a log level ≤ LOG_LEVEL are printed. Default:
 *LOG_LEVEL_DBG (all messages enabled).el can be adjusted to control the
 *verbosity of logging output.
 * Example:
 * \code
 * #define LOG_LEVEL LOG_LEVEL_INF  // Show errors, warnings, and info.
 * \endcode
 *
 * \see StatusbarLog::log: Actual function used for creating log messages.
 * \see StatusbarLog::log_level: Enum containing all log levels.
 */
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DBG
#endif  // !LOG_LEVEL

namespace StatusbarLog {

typedef struct {
  std::size_t idx;
  unsigned int ID;
  bool valid;
} StatusBar_handle;



/**
 * \brief Saves the current cursor position in the terminal
 */
void save_cursor_position();

/**
 * \brief Restores the previously saved cursor position in the terminal
 */
void restore_cursor_position();

/**
 * \brief Clears from the current cursor position to the end of the line
 */
void clear_to_end_of_line();

/**
 * \brief Clears from the current cursor position to the end of the line
 */
void clear_from_start_of_line();

/**
 * \brief Clear entire current line
 */
void clear_line();

/**
 * \brief More robust version with cursor positioning
 */
void clear_current_line();

/**
 * \brief \brief Logs a message if its level ≤ LOG_LEVEL
 *
 * \param[in] filename Source filename or tag (will be printed in log message,
 * should be the origin of the log message).
 * \param[in] fmt printf-style format string.
 * \param[in] log_level Severity level of this message.
 * \param[in] ... Additional format arguments.
 *
 * \return 0 on success, non-zero on formatting error. (Currently always 0, no
 * error checking)
 *
 * \note Logging temporarily moves status bars down to avoid visual glitches.
 *
 * \see StatusbarLog::log_level: Enum containing all log levels.
 * \see StatusbarLog::log_LEVEL: Macro to set the global logging threshold.
 * \see print_err: Function for printing error messages -> \todo
 */
int log(const std::string& filename, const std::string& fmt,
        Log_level log_level, ...);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

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
#pragma GCC diagnostic pop

/**
 * \brief Initializes a StatusBar, updates its handle and prints its initial
 * state.
 *
 * This function takes an empty statusbar_handle struct and populate its
 * corresponding statusbar with the other input parameters. Additionally it sets
 * all percentages to 0 and sets the spinner-indices to 0.
 *
 * Example of a statusbar:
 *  "prefix1 string"[########/       ] 50% "postfix1 string"
 *  "prefix2 string"[#/        ] 10% "postfix2 string"
 *
 * \param[out] statusbar_handle Struct to initialize.
 * \param[in] _positions Vertical positions (1=topmost) of each bar. For e.g. if
 * you want two bars stacked on top of each other you would pass {2, 1} (2: top
 * bar, 1: lower bar).
 * \param[in] _bar_sizes Widths of each bar (characters excluding prefix,
 * postfix, percentage, '[' and '[').
 * \param[in] _prefixes Text before each bar.
 * \param[in] _postfixes Text before each bar.
 *
 * \warning Add the StatusBar* to g_statusbar_registry after creation.
 * \warning Remove from g_statusbar_registry before destruction.
 *
 * \see g_statusbar_registry: Global statusbar registry.
 * \see StatusBar: The statusbar struct.
 * \see update_statusbar: Updating a statusbar
 */
int create_statusbar_handle(StatusBar_handle& statusbar,
                            const std::vector<unsigned int> _positions,
                            const std::vector<unsigned int> _bar_sizes,
                            const std::vector<std::string> _prefixes,
                            const std::vector<std::string> _postfixes);
int destroy_statusbar_handle(StatusBar_handle& statusbar_handle);

/**
 * \brief Function used for updating a statusbar given its handle. The statusbar
 * can consist of multiple "bars" of different sizes and different post-, and
 * prefixes.
 *
 * This Function takes a StatusBar_handle struct and 'updates' it by printing
 * the bar given a new percentage. The statusbar can consist of multiple bars
 * which is why an index has to be passed. This function moves the cursor to the
 * correct location in the terminal corresponding to the index, clears the row
 * and prints an updated bar.
 *
 * Example of a statusbar:
 *  "prefix1 string"[########/       ] 50% "postfix1 string"
 *  "prefix2 string"[#/        ] 10% "postfix2 string"
 *
 *
 * \param[in, out] statusbar_handle Statusbar to update.
 * \param[in] idx Index of the bar component (0-based).
 * \param[in] percent New progress percentage (0-100).
 * updated.
 *
 * \details The spinner character cycles through { |, /, -, \ } on each update.
 *
 * \see create_statusbar: Creating/Initializing a statusbar.
 * \see StatusBar: The statusbar struct.
 * \see g_statusbar_registry: Global statusbar registry.
 */
int update_statusbar(StatusBar_handle& statusbar, const std::size_t idx,
                     const double percent);

}  // namespace StatusbarLog

#endif  // !STATUSBARLOG_H
