#include <cstddef>
#include <string>
#include <vector>
#define LOG_LEVEL LOG_LEVEL_INF
#include <iostream>
#include <thread>

#include "StatusbarLog.h"

#define FILENAME "main.cpp"

int main() {
  int err = 0;
  const int total_steps1 = 50;
  const int total_steps2 = 100;

  StatusbarLog::ProgressBar progressbar;
  // const std::vector<unsigned int> positions = {2, 1};
  // const std::vector<unsigned int> bar_sizes = {20, 10};
  // const std::vector<std::string> prefixes = {"first : ", "second: "};
  // const std::vector<std::string> postfixes = {"50 total steps", "100 total
  // steps"}; err = StatusbarLog::create_progressbar(progressbar, positions,
  // bar_sizes, prefixes,
  //                                  postfixes);
  err = StatusbarLog::create_progressbar(
      progressbar, {2, 1},                                     // <-- Postions
      {20, 10},                                                // <-- Bar widths
      {"first:  ", "second: "},                                // <-- prefixes
      {" -- 50 total stpes", "           -- 100 total steps"}  // <-- postfixes
  );
  if (err != 0) {
    LOG_ERR(FILENAME, "Failed to create statusbar. Errorcode %d", err);
    return err;
  }

  for (int i = 0; i <= total_steps1; ++i) {
    double percent = static_cast<double>(i) / total_steps1 * 100;
    StatusbarLog::update_progress_bar(progressbar, 0, percent);
    if (i % 100 == 0) {
      // LOG_INF("main.cpp", "100 Ticks reached\n");
    }

    // Simulate work:
    for (int j = 0; j <= total_steps2; ++j) {
      double percent = static_cast<double>(j) / total_steps2 * 100;
      StatusbarLog::update_progress_bar(progressbar, 1, percent);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  // Ensure the bar ends with newline when complete
  std::cout << std::endl;
  return 0;
}
