// -- StatusbarLog/src/StatusbarLog.cpp

#include "StatusbarLog.h"

#include <array>
#include <cmath>
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

}  // namespace

namespace StatusbarLog {

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

int draw_progress_bar(double percent, int bar_width, std::size_t spinner_idx,
                      int move) {
  static const std::array<char, 4> spinner = {'|', '/', '-', '\\'};
  char spin_char = spinner[spinner_idx % spinner.size()];

  int fill = std::floor((percent * static_cast<double>(bar_width)) / 100.0);
  int empty = bar_width - fill;

  std::ostringstream oss;
  oss << "[";
  oss << std::string(fill, '#');
  if (bar_width > fill) {
    oss << spin_char;
    oss << std::string(empty - 1, ' ');
  } else {
    oss << std::string(empty, ' ');
  }
  oss << "] ";
  oss << std::fixed << std::setprecision(2) << std::setw(5) << percent;
  _move_cursor_up(move);
  std::cout << oss.str() << std::flush;
  _move_cursor_up(-move);

  return 0;
}

};  // namespace StatusbarLog
