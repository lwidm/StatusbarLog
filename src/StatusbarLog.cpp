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

void _move_cursor_up(int move) {
  if (move > 0) {
    std::cout << "\033[" << move << "A";
  } else if (move < 0) {
    std::cout << std::string(-move, '\n');
  }
  std::cout << std::flush;
}

int _draw_progress_bar_component(const double percent,
                                 const unsigned int bar_width,
                                 const std::string prefix,
                                 const std::string postfix,
                                 std::size_t& spinner_idx, const int move) {
  static const std::array<char, 4> spinner = {'|', '/', '-', '\\'};
  char spin_char = spinner[spinner_idx % spinner.size()];

  const unsigned int fill = std::floor((percent * static_cast<double>(bar_width)) / 100.0);
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

std::vector<ProgressBar> progressbars = {};

int log(const std::string& filename, const std::string& fmt,
        const Log_level log_level, ...) {
  if (log_level > LOG_LEVEL) return 0;

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

  va_list args;
  va_start(args, log_level);
  printf("%s [%s]: ", prefix, filename.c_str());
  vprintf(fmt.c_str(), args);
  printf("\n");
  va_end(args);
  return 0;
}

int create_progressbar(ProgressBar& progressbar,
                       const std::vector<unsigned int> _positions,
                       const std::vector<unsigned int> _bar_sizes,
                       const std::vector<std::string> _prefixes,
                       const std::vector<std::string> _postfixes) {
  progressbar = {_positions, _bar_sizes, _prefixes, _postfixes,
                 std::vector<std::size_t>{0}};
  const unsigned int num_bars = progressbars.size();
  for (std::size_t idx = 0; idx < num_bars; idx++) {
    _draw_progress_bar_component(
        0.0, progressbar.bar_sizes[idx], progressbar.prefixes[idx],
        progressbar.postfixes[idx], progressbar.spin_idxs[idx],
        progressbar.positions[idx]);
    return 0;
  }
  return 0;
}

int update_progress_bar(ProgressBar& progressbar, const std::size_t idx,
                        const double percent) {
  _draw_progress_bar_component(
      percent, progressbar.bar_sizes[idx], progressbar.prefixes[idx],
      progressbar.postfixes[idx], progressbar.spin_idxs[idx],
      progressbar.positions[idx]);
  return 0;
}

};  // namespace StatusbarLog
