#include <cstddef>
#define LOG_LEVEL LOG_LEVEL_INF
#include "StatusbarLog.h"
#include <thread>
#include <iostream>

int main() {
  const int total_steps = 10000;
  const int bar_width = 40;
  std::size_t spinner_idx = 0;

  for (int i = 0; i <= total_steps; ++i) {
    double percent = static_cast<double>(i)/total_steps * 100;
    StatusbarLog::draw_progress_bar(percent, bar_width, spinner_idx);
    spinner_idx++;
    if (i % 100 == 0){
      LOG_INF("main.cpp", "100 Ticks reached");
    }

    // Simulate work:
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Ensure the bar ends with newline when complete
  std::cout << std::endl;
  return 0;
}
