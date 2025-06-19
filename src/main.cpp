#include <cstddef>
#include <string>
#include <vector>
#define LOG_LEVEL LOG_LEVEL_INF
#include <iostream>
#include <thread>

#include "StatusbarLog.h"

#define FILENAME "main.cpp"

void print_with_cleanup() {
  std::cout << "Start to be kept <- " << std::flush;;
  StatusbarLog::save_cursor_position();
  std::cout << "Temporary message that might be long" << std::flush;;
  std::this_thread::sleep_for(std::chrono::seconds(2));

  StatusbarLog::restore_cursor_position();
  StatusbarLog::clear_to_end_of_line();
  std::cout << "Clean message" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
  print_with_cleanup();
  LOG_INF(FILENAME, "Starting test...");
  LOG_INF(FILENAME, "Starting test...");
  LOG_INF(FILENAME, "Starting test...");
  LOG_INF(FILENAME, "Starting test...");

  int err = 0;
  const int total_steps1 = 15;
  const int total_steps2 = 100;

  StatusbarLog::StatusBar_handle h;

  std::cout << "\n\n";
  err = StatusbarLog::create_statusbar_handle(
      h, {2, 1},                                               // <-- Postions
      {20, 1000},                                              // <-- Bar widths
      {"first:  ", "second: "},                                // <-- prefixes
      {" -- 15 total steps", "           -- 100 total steps"}  // <-- postfixes
  );
  if (err != 0) {
    LOG_ERR(FILENAME, "Failed to create statusbar. Errorcode %d", err);
    return err;
  }

  for (std::size_t i = 0; i <= total_steps1; ++i) {
    double percent = static_cast<double>(i) / total_steps1 * 100.0;
    StatusbarLog::update_statusbar(h, 0, percent);
    if (i % 10 == 0) {
      LOG_INF("main.cpp", "10 Ticks reached\n");
    }

    // Simulate work:
    for (std::size_t j = 0; j <= total_steps2; ++j) {
      double percent = static_cast<double>(j) / total_steps2 * 100.0;
      StatusbarLog::update_statusbar(h, 1, percent);
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
  }

  err = StatusbarLog::destroy_statusbar_handle(h);
  if (err != 0) {
    LOG_ERR(FILENAME, "Failed to destroy statusbar. Errorcode %d", err);
    return err;
  }

  return 0;
}
