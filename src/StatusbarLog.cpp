// -- StatusbarLog/src/StatusbarLog.cpp

#include <cstdarg>
#include <cstdio>
#include <string>

#include "StatusbarLog.h"

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

};  // namespace StatusbarLog
