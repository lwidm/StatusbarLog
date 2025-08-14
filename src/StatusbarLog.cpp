// -- StatusbarLog/src/StatusbarLog.cpp

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <vector>

#include "StatusbarLog.h"

#define FILENAME "StatusbarLog.cpp"

namespace StatusbarLog {

// Hidden implementation detail
namespace {

/**
 * \struct StatusBar
 * \brief Represents a multi-component status bar with progress indicators.
 *
 * A status bar can contain multiple stacked bars, each with:
 * - Progress percentages (0-100) for each bar.
 * - Vertical positions (1=topmost).
 * - Total width (characters) of each bar.
 * - Text displayed before each bar.
 * - Text displayed after each bar.
 * - Spinner animation indices.
 * - Indicator whether error already has been reported
 * - unique ID corresponding to the handle
 */
// clang-format off
typedef struct {
  std::vector<double> percentages;      ///< Progress percentages (0-100) for each bar.
  std::vector<unsigned int> positions;  ///< Vertical positions (1=topmost).
  std::vector<unsigned int> bar_sizes;  ///< Total width (characters) of each bar.
  std::vector<std::string> prefixes;    ///< Text displayed before each bar.
  std::vector<std::string> postfixes;   ///< Text displayed after each bar.
  std::vector<std::size_t> spin_idxs;   ///< Spinner animation indices.
  bool error_reported;                  ///< Indicator whether error already has been reported
  unsigned int ID;                      ///< unique ID corresponding to the handle
} StatusBar;
// clang-format on

std::vector<StatusBar> statusbar_registry;
std::vector<StatusBar_handle> statusbar_free_handles;
unsigned int handle_ID_count = 0;

/**
 * \brief Function used only by the StatusbarLog module to move the cursor up X
 * lines in the standard output terminal
 *
 * \param[in] move: How many lines to move upwards. Negative values mean moving
 * down.
 *
 */
void _move_cursor_up(int move) {
  if (move > 0) {
    std::cout << "\033[" << move << "A";
  } else if (move < 0) {
    std::cout << std::string(-move, '\n');
  }
  std::cout << std::flush;
}

/**
 * \brief Gets terminal width in columns.
 *
 * \param[out] width Receives terminal width. Defaults to 80 on failure.
 *
 * \return 0 on success
 * \return -1 (Windows) or -2 (Unix) on failure
 *
 */
int _get_terminal_width(int& width) {
  width = 80;  // Default value

#ifdef _WIN32
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsole != INVALID_HANDLE_VALUE) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
      width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    } else {
      return -1;
    }
  }
#else
  winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
    width = w.ws_col;
  } else {
    return -2;
  }
#endif
  return 0;
}

/**
 * \brief Check if the argument is a valid statusbar handle
 *
 * This functions performs a test on a StatusBar_handle and returns 0 if
 * it is a valid handle and a negative number otherwise
 *
 * \param[in] StatusBar_handle struct to be checked for validity
 *
 * \return Returns 0 if the handle is valid, or one of these status codes:
 *         -  0: Valid handle
 *         - -1: Invalid handle: Valid flag of handle set to false
 *         - -2: Invalid handle: Handle index out of bounds in
 * `statusbar_registry`
 *         - -3: Invalid handle: Handle IDs don't match between handle struct
 * and registry
 *
 * \see _is_valid_handle: Verbose version of this function
 */
int _is_valid_handle(const StatusBar_handle& statusbar_handle) {
  const std::size_t idx = statusbar_handle.idx;

  if (!statusbar_handle.valid) {
    return -1;
  }

  if (statusbar_handle.idx >= statusbar_registry.size()) {
    return -2;
  }
  if (statusbar_handle.ID != statusbar_registry[idx].ID) {
    return -3;
  }
  return 0;
}

/**
 * \brief Check if the argument is a valid statusbar handle and prints an error
 * message.
 *
 * This functions performs a test on a StatusBar_handle and returns 0 if
 * it is a valid handle and a negative number otherwise. Same as
 * _is_valid_handle but also prints an error message and returns the target
 * StatusBar
 *
 * \param[in] StatusBar_handle struct to be checked for validity
 *
 * \return Returns 0 if the handle is valid, or one of these status codes:
 *         -  0: Valid handle
 *         - -1: Invalid handle: Valid flag of handle set to false
 *         - -2: Invalid handle: Handle index out of bounds in
 * `statusbar_registry`
 *         - -3: Invalid handle: Handle IDs don't match between handle struct
 * and registry
 *
 * \see _is_valid_handle: Non verbose version of this function
 */
int _is_valid_handle_verbose(const StatusBar_handle& statusbar_handle) {
  const int is_valid_handle = _is_valid_handle(statusbar_handle);
  if (is_valid_handle == -1) {
    LOG_WRN(FILENAME,
            "Invalid handle: Valid flag set to false (idx: %zu, ID: %u)",
            statusbar_handle.idx, statusbar_handle.ID);
    return -1;
  }

  if (is_valid_handle == -2) {
    LOG_WRN(FILENAME,
            "Invalid Handle: Handle index %zu out of bounds (max %zu)",
            statusbar_handle.idx, statusbar_registry.size());
    return -2;
  }

  StatusBar& target = statusbar_registry[statusbar_handle.idx];

  if (is_valid_handle == -3) {
    LOG_WRN(FILENAME, "Invalid Handle: ID mismatch: handle %u vs registry %u",
            statusbar_handle.ID, target.ID);
    return -3;
  }

  if (is_valid_handle != 0) {
    LOG_WRN(FILENAME, "Invalid Handle: Errorcode not handled!");
    return -4;
  }

  return 0;
}

/**
 * \brief Function used only by that StatusbarLog module to draw a single status
 * bar at a certain position.
 *
 * This function draws a single status bar (not multiple stacked ones) from
 * primitive variables.
 *
 * Example single statusbar:
 * "prefix string"[########/       ] 50% "postfix string"
 *
 * the bar can be drawn at an arbitrary postion above or on the cursur using the
 * `move` parameter.
 *
 * \param[in] percent: Progress percentage (0-100).
 * \param[in] bar_width: Total bar width (characters excluding prefix, postfix,
 * percentage, '[' and '[').
 * \param[in] prefix: Text before the bar.
 * \param[in] postfix: Text after the bar.
 * \param[in, out] spinner_idx: Index for spinner animation (incremented on
 * call).
 * \param[in] move: Vertical offset from cursor (positive = up).
 *
 * \details Using the spinner_idx the spinner character can cycle through { |,
 * /, -, \ } on each update.
 *
 * \return Returns 0 on success, or one of these error/warning codes:
 *         -  0: Success (no errors)
 *         - -1: Terminal width detection failed (Windows)
 *         - -2: Terminal width detection failed (Linux)
 *         - -3: Truncantion was needed (bar exeeds terminal width)
 *         - -4: Both terminal width detection failed (Window) AND truncation
 * was needed
 *         - -5: Both terminal width detection failed (Linux) AND truncation was
 * needed
 */
int _draw_statusbar_component(const double percent,
                              const unsigned int bar_width,
                              const std::string& prefix,
                              const std::string& postfix,
                              std::size_t& spinner_idx, const int move) {
  int err = 0;
  static const std::array<char, 4> spinner = {'|', '/', '-', '\\'};
  spinner_idx %= spinner.size();
  char spin_char = spinner[spinner_idx];

  const unsigned int fill =
      std::floor((percent * static_cast<double>(bar_width)) / 100.0);
  const unsigned int empty = bar_width - fill;

  std::ostringstream oss;
  oss << prefix;
  oss << "[";
  oss << std::string(fill, '#');
  if (empty > 0) {
    oss << spin_char;
    oss << std::string(empty - 1, ' ');
  } else {
    oss << std::string(empty, ' ');
  }
  oss << "] ";
  oss << std::fixed << std::setprecision(2) << std::setw(6) << percent;
  oss << postfix;
  std::string status_str = oss.str();

  int term_width;
  err = _get_terminal_width(term_width);

  if (status_str.length() > static_cast<size_t>(term_width)) {
    status_str = status_str.substr(0, term_width - 1);
    switch (err) {
      case 0:
        err = -3;
        break;
      case -1:
        err = -4;
        break;
      case -2:
        err = -5;
        break;
    }
  }

  _move_cursor_up(move);
  clear_current_line();
  std::cout << status_str << std::flush;
  _move_cursor_up(-move);

  return err;
}

}  // namespace

void save_cursor_position() {
  std::cout << "\033[s"
            << std::flush;  // ANSI escape code to save cursor position
}

void restore_cursor_position() {
  std::cout << "\033[u"
            << std::flush;  // ANSI escape code to restore cursor position
}

void clear_to_end_of_line() {
  std::cout << "\033[0K"
            << std::flush;  // ANSI escape code to clear to end of line
}

void clear_from_start_of_line() {
  std::cout << "\033[1K"
            << std::flush;  // ANSI escape code to clear to end of line
}

void clear_line() { std::cout << "\033[2K" << std::flush; }

void clear_current_line() {
  std::cout << "\r"       // Return to line start
            << "\033[2K"  // Clear entire line
            << std::flush;
}

int log(const Log_level log_level, const std::string& filename, const char* fmt,
        ...) {
  if (log_level > LOG_LEVEL) return 0;
  const bool statusbars_active = !statusbar_registry.empty();

  const char* prefix = "";
  // clang-format off
  switch(log_level){
    case LOG_LEVEL_ERR: prefix = "ERROR"; break;
    case LOG_LEVEL_WRN: prefix = "WARNING"; break;
    case LOG_LEVEL_INF: prefix = "INFO"; break;
    case LOG_LEVEL_DBG: prefix = "DEBUG"; break;
    default: break;
  }
  // clang-format on

  int move = 0;
  if (statusbars_active) {
    for (std::size_t i = 0; i < statusbar_registry.size(); ++i) {
      for (std::size_t j = 0; j < statusbar_registry[i].positions.size(); ++j) {
        int current_pos = statusbar_registry[i].positions[j];
        if (current_pos > move) {
          move = current_pos;
        }
      }
    }
  }

  va_list args;
  va_list args_copy;
  _move_cursor_up(move);
  if (statusbars_active) printf("\r\033[2K\r");
  va_start(args, fmt);
  va_copy(args_copy, args);
  const int size = std::vsnprintf(nullptr, 0, fmt, args_copy);
  std::vector<char> buffer(size + 1);
  std::vsnprintf(buffer.data(), buffer.size(), fmt, args);
  printf("%s [%s]: %s\n", prefix, filename.c_str(), buffer.data());
  _move_cursor_up(-move);
  va_end(args);

  if (statusbars_active) {
    for (std::size_t i = 0; i < statusbar_registry.size(); ++i) {
      for (std::size_t j = 0; j < statusbar_registry[i].positions.size(); ++j) {
        _draw_statusbar_component(statusbar_registry[i].percentages[j],
                                  statusbar_registry[i].bar_sizes[j],
                                  statusbar_registry[i].prefixes[j],
                                  statusbar_registry[i].postfixes[j],
                                  statusbar_registry[i].spin_idxs[j],
                                  statusbar_registry[i].positions[j]);
      }
    }
  }
  return 0;
}

int create_statusbar_handle(StatusBar_handle& statusbar_handle,
                            const std::vector<unsigned int> _positions,
                            const std::vector<unsigned int> _bar_sizes,
                            const std::vector<std::string> _prefixes,
                            const std::vector<std::string> _postfixes) {
  if (_positions.size() != _bar_sizes.size() ||
      _bar_sizes.size() != _prefixes.size() ||
      _prefixes.size() != _positions.size()) {
    LOG_ERR(FILENAME,
            "The vecotors '_positions', '_bar_sizes', '_prefixes' and "
            "'_postfixes' must have the same size! Got: '_positions': %d, "
            "'_bar_sizes': %d, '_prefixes': %d, '_postfixes': %d.",
            _positions.size(), _bar_sizes.size(), _prefixes.size(),
            _postfixes.size());
    return -1;
  }

  handle_ID_count++;
  const std::size_t num_bars = _positions.size();
  const std::vector<double> percentages(num_bars, 0.0);
  const std::vector<std::size_t> spin_idxs(num_bars, 0);

  if (!statusbar_free_handles.empty()) {
    StatusBar_handle free_handle = statusbar_free_handles.back();
    statusbar_free_handles.pop_back();
    statusbar_handle.idx = free_handle.idx;
    statusbar_registry[statusbar_handle.idx] = {
        percentages, _positions, _bar_sizes, _prefixes,
        _postfixes,  spin_idxs,  false,      handle_ID_count};
  } else {
    statusbar_handle.idx = statusbar_registry.size();
    statusbar_registry.emplace_back(
        StatusBar{percentages, _positions, _bar_sizes, _prefixes, _postfixes,
                  spin_idxs, false, handle_ID_count});
  }
  statusbar_handle.ID = handle_ID_count;
  statusbar_handle.valid = true;
  for (std::size_t idx = 0; idx < num_bars; idx++) {
    _draw_statusbar_component(
        0.0, statusbar_registry[statusbar_handle.idx].bar_sizes[idx],
        statusbar_registry[statusbar_handle.idx].prefixes[idx],
        statusbar_registry[statusbar_handle.idx].postfixes[idx],
        statusbar_registry[statusbar_handle.idx].spin_idxs[idx],
        statusbar_registry[statusbar_handle.idx].positions[idx]);
  }
  return 0;
}

int destroy_statusbar_handle(StatusBar_handle& statusbar_handle) {
  const int err = _is_valid_handle_verbose(statusbar_handle);
  if (err != 0) {
    LOG_ERR(FILENAME, "Failed to destory statusbar_handle!");
    return err;
  }
  StatusBar& target = statusbar_registry[statusbar_handle.idx];

  for (std::size_t i = 0; i < target.positions.size(); i++) {
    _move_cursor_up(target.positions[i]);
    clear_current_line();
    _move_cursor_up(-target.positions[i]);
  }
  std::cout.flush();

  target.percentages.clear();
  target.positions.clear();
  target.bar_sizes.clear();

  for (std::string& str : target.prefixes) {
    std::fill(str.begin(), str.end(), '\0');
  }
  for (std::string& str : target.postfixes) {
    std::fill(str.begin(), str.end(), '\0');
  }
  target.prefixes.clear();
  target.postfixes.clear();

  target.ID = 0;
  target.spin_idxs.clear();

  statusbar_free_handles.push_back(statusbar_handle);

  statusbar_handle.valid = false;
  statusbar_handle.ID = 0;
  statusbar_handle.idx = SIZE_MAX;

  return 0;
}

int update_statusbar(StatusBar_handle& statusbar_handle, const std::size_t idx,
                     const double percent) {
  const int err = _is_valid_handle_verbose(statusbar_handle);
  if (err != 0) {
    LOG_ERR(FILENAME, "Failed to update statusbar_handle!");
    return err;
  }
  StatusBar& statusbar = statusbar_registry[statusbar_handle.idx];

  statusbar.percentages[idx] = percent;
  statusbar.spin_idxs[idx] = statusbar.spin_idxs[idx] + 1;
  int bar_error_code = _draw_statusbar_component(
      percent, statusbar.bar_sizes[idx], statusbar.prefixes[idx],
      statusbar.postfixes[idx], statusbar.spin_idxs[idx],
      statusbar.positions[idx]);

  if (bar_error_code < 0 && !statusbar.error_reported) {
    statusbar.error_reported = true;
    const char* why;
    switch (bar_error_code) {
      case -1:
        why = "Terminal width detection failed (Windows)";
        break;
      case -2:
        why = "Terminal width detection failed (Linux)";
        break;
      case -3:
        why = "Truncating was needed";
        break;
      case -4:
        why =
            "Terminal width detection failed (Windows) and truncation was "
            "needed";
        break;
      case -5:
        why =
            "Terminal width detection failed (Linux) and truncation was "
            "needed";
        break;
    }
    LOG_ERR(FILENAME, "%s on statusbar with ID %u at bar idx %zu!", why,
            statusbar.ID, idx);
  }

  return 0;
}

};  // namespace StatusbarLog
