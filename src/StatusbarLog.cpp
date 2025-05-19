// -- StatusbarLog/src/StatusbarLog.cpp

#include "StatusbarLog.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <ostream>

// Hidden implementation detail
namespace {

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
 * \param[in, out] spinner_idx: Index for spinner animation (incremented on call).
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

namespace StatusbarLog {

std::vector<StatusBar*> g_statusbar_registry = {};

int log(const std::string& filename, const std::string& fmt,
        const Log_level log_level, ...) {
  if (log_level > LOG_LEVEL) return 0;
  const bool statusbars_active = !g_statusbar_registry.empty();

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

  unsigned int move = 0;
  if (statusbars_active) {
    for (std::size_t i = 0; i < g_statusbar_registry.size(); ++i) {
      for (std::size_t j = 0; j < g_statusbar_registry[i]->positions.size();
           ++j) {
        unsigned int current_pos = g_statusbar_registry[i]->positions[j];
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
    for (std::size_t i = 0; i < g_statusbar_registry.size(); ++i) {
      for (std::size_t j = 0; j < g_statusbar_registry[i]->positions.size();
           ++j) {
        update_statusbar(*g_statusbar_registry[i], j,
                         g_statusbar_registry[i]->percentages[j]);
      }
    }
  } else {
    printf("\n");
  }

  return 0;
}

int create_statusbar(StatusBar& statusbar,
                     const std::vector<unsigned int> _positions,
                     const std::vector<unsigned int> _bar_sizes,
                     const std::vector<std::string> _prefixes,
                     const std::vector<std::string> _postfixes) {
  std::vector<double> percentages(_positions.size(), 0.0);
  statusbar = {percentages, _positions, _bar_sizes,
               _prefixes,   _postfixes, std::vector<std::size_t>{0}};
  const unsigned int num_bars = g_statusbar_registry.size();
  for (std::size_t idx = 0; idx < num_bars; idx++) {
    _draw_statusbar_component(0.0, statusbar.bar_sizes[idx],
                              statusbar.prefixes[idx], statusbar.postfixes[idx],
                              statusbar.spin_idxs[idx],
                              statusbar.positions[idx]);
    return 0;
  }
  return 0;
}

int update_statusbar(StatusBar& statusbar, const std::size_t idx,
                     const double percent) {
  statusbar.percentages[idx] = percent;
  _draw_statusbar_component(percent, statusbar.bar_sizes[idx],
                            statusbar.prefixes[idx], statusbar.postfixes[idx],
                            statusbar.spin_idxs[idx], statusbar.positions[idx]);
  return 0;
}

};  // namespace StatusbarLog
