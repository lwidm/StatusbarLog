// -- StatusbarLog/src/StatusbarLog.cpp

#include "StatusbarLog.h"

#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <ostream>

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

int draw_progress_bar(double percent, int bar_width, std::size_t spinner_idx) {
  static const std::array<char, 4> spinner = {'|', '/', '-', '\\'};
  char spin_char = spinner[spinner_idx % spinner.size()];

  int fill = std::floor((percent * static_cast<double>(bar_width)) / 100.0);
  int empty = bar_width - fill;

  std::string bar = "[";
  bar += std::string(fill, '#');
  if (bar_width > fill) {
    bar += spin_char;
    bar += std::string(empty - 1, ' ');
  } else {
    bar += std::string(empty, ' ');
  }
  bar += "] ";
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << std::setw(5) << percent;
  bar += oss.str();
  std::cout << bar << '\r' << std::flush;

  return 0;
}

};  // namespace StatusbarLog
