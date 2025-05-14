#define LOG_LEVEL LOG_LEVEL_ERR
#include "StatusbarLog.h"

int main () {
  StatusbarLog::log("main.cpp", "hello world %.4f, %s, %.3e", LOG_LEVEL_DBG, 123.2134142412412, "hi", 123123e-3);
  return 0;
}
