// -- statusbarlog/test/src/statusbarlog_main.cpp

#include <cstddef>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#define LOG_LEVEL statusbar_log::kLogLevelInf

#include "statusbarlog/statusbarlog.h"

#define FILENAME "main.cpp"

void print_with_cleanup() {
  std::cout << "Start to be kept <- " << std::flush;
  ;
  statusbar_log::save_cursor_position();
  std::cout << "Temporary message that might be long" << std::flush;
  ;
  std::this_thread::sleep_for(std::chrono::seconds(2));

  statusbar_log::restore_cursor_position();
  statusbar_log::clear_to_end_of_line();
  std::cout << "Clean message" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
  print_with_cleanup();
  LOG_INF(FILENAME, "Starting test...");
  LOG_INF(FILENAME, "Starting test...");
  LOG_INF(FILENAME, "Starting test...");
  LOG_INF(FILENAME, "Starting test...");
  // LOG_INF("ExremelyLongFilenameWhichShouldBeTruncated", "Lorem ipsum dolor
  // sit amet consectetur adipiscing elit. Quisque faucibus ex sapien vitae
  // pellentesque sem placerat. In id cursus mi pretium tellus duis convallis.
  // Tempus leo eu aenean sed diam urna tempor. Pulvinar vivamus fringilla lacus
  // nec metus bibendum egestas. Iaculis massa nisl malesuada lacinia integer
  // nunc posuere. Ut hendrerit semper vel class aptent taciti sociosqu. Ad
  // litora torquent per conubia nostra inceptos himenaeos.");

  int err = 0;
  const int total_steps1 = 15;
  const int total_steps2 = 100;

  statusbar_log::StatusbarHandle h;

  std::cout << "\n\n";
  err = statusbar_log::create_statusbar_handle(
      h, {2, 1},                                               // <-- Postions
      {20, 10},                                                // <-- Bar widths
      {"first:  ", "second: "},                                // <-- prefixes
      {" -- 15 total steps", "           -- 100 total steps"}  // <-- postfixes
  );
  if (err != 0) {
    LOG_ERR(FILENAME, "Failed to create statusbar. Errorcode %d", err);
    return err;
  }

  for (std::size_t i = 0; i <= total_steps1; ++i) {
    double percent = static_cast<double>(i) / total_steps1 * 100.0;
    statusbar_log::update_statusbar(h, 0, percent);
    if (i % 10 == 0 && i != 0) {
      LOG_INF(FILENAME, "10 Ticks reached");
    }
    if (i % 3 == 0 && i != 0) {
      // statusbar_log::update_statusbar(h, 0, 100.1);
      // statusbar_log::update_statusbar(h, 0, -0.1);
    }

    // Simulate work:
    for (std::size_t j = 0; j <= total_steps2; ++j) {
      double percent = static_cast<double>(j) / total_steps2 * 100.0;
      statusbar_log::update_statusbar(h, 1, percent);
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
  }

  err = statusbar_log::destroy_statusbar_handle(h);
  if (err != 0) {
    LOG_ERR(FILENAME, "Failed to destroy statusbar. Errorcode %d", err);
    return err;
  }

  return 0;
}
