// -- StatusbarLog/src/StatusbarLog.cpp

#include "StatusbarLog.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <ostream>

#define FILENAME "StatusbarLog.cpp"

namespace StatusbarLog {

// Hidden implementation detail
namespace {

/**
 * \struct StatusBar
 * \brief Represents a multi-component status bar with progress indicators.
 *
 * A status bar can contain multiple stacked bars, each with:
 * - A position (vertical order).
 * - A width (number of characters).
 * - Prefix/postfix text.
 * - A progress percentage.
 * - A spinner animation index.
 */
// clang-format off
typedef struct {
  std::vector<double> percentages;      ///< Progress percentages (0-100) for each bar.
  std::vector<unsigned int> positions;  ///< Vertical positions (1=topmost).
  std::vector<unsigned int> bar_sizes;  ///< Total width (characters) of each bar.
  std::vector<std::string> prefixes;    ///< Text displayed before each bar.
  std::vector<std::string> postfixes;   ///< Text displayed after each bar.
  std::vector<std::size_t> spin_idxs;   ///< Spinner animation indices.
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
 */
int _draw_statusbar_component(const double percent,
                              const unsigned int bar_width,
                              const std::string prefix,
                              const std::string postfix,
                              std::size_t& spinner_idx, const int move) {
  static const std::array<char, 4> spinner = {'|', '/', '-', '\\'};
  char spin_char = spinner[spinner_idx % spinner.size()];

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
  _move_cursor_up(move);
  std::cout << oss.str() << std::flush;
  _move_cursor_up(-move);
  spinner_idx++;

  return 0;
}

}  // namespace

int log(const std::string& filename, const std::string& fmt,
        const Log_level log_level, ...) {
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
  _move_cursor_up(move);
  if (statusbars_active) printf("\r\033[2K\r");
  va_start(args, log_level);
  printf("%s [%s]: ", prefix, filename.c_str());
  vprintf(fmt.c_str(), args);
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
  } else {
    printf("\n");
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
  const std::vector<std::size_t> spin_idxs(num_bars, 0.0);

  if (!statusbar_free_handles.empty()) {
    StatusBar_handle free_handle = statusbar_free_handles.back();
    statusbar_free_handles.pop_back();
    statusbar_handle.idx = free_handle.idx;
    statusbar_registry[statusbar_handle.idx] = {
        percentages, _positions, _bar_sizes,     _prefixes,
        _postfixes,  spin_idxs,  handle_ID_count};
  } else {
    statusbar_handle.idx = statusbar_registry.size();
    statusbar_registry.emplace_back(StatusBar{percentages, _positions, _bar_sizes,
                                    _prefixes, _postfixes, spin_idxs,
                                    handle_ID_count});
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

int update_statusbar(StatusBar_handle& statusbar_handle, const std::size_t idx,
                     const double percent) {
  if (!statusbar_handle.valid) {
    LOG_ERR(FILENAME, "Invalid hanldes cannot be updated!");
    return -1;
  }
  StatusBar statusbar = statusbar_registry[statusbar_handle.idx];
  if (statusbar_handle.ID != statusbar.ID) {
    LOG_ERR(FILENAME, "Handle and statusbar IDs do not match! Got %d and %d",
            statusbar_handle.ID, statusbar.ID);
    return -2;
  }
  if (idx >= statusbar.percentages.size()) {
    LOG_ERR(FILENAME, "Index (%d) out of bounds for given statusbar of size %d",
            idx, statusbar.percentages.size());
    return -3;
  }

  statusbar.percentages[idx] = percent;
  _draw_statusbar_component(percent, statusbar.bar_sizes[idx],
                            statusbar.prefixes[idx], statusbar.postfixes[idx],
                            statusbar.spin_idxs[idx], statusbar.positions[idx]);
  return 0;
}

};  // namespace StatusbarLog
