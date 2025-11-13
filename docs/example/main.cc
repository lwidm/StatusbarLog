// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 Lukas Widmer

// -- statusbarlog/test/src/statusbarlog_main.cc

#include <cstddef>
#include <iostream>
#include <string>
#include <thread>

#include "statusbarlog/statusbarlog.h"

const std::string kFilename = "main.cc";

int main() {
  std::cout << "Start to be kept <- " << std::flush;
  statusbar_log::SaveCursorPosition();
  std::cout << "Temporary message that might be long" << std::flush;
  std::this_thread::sleep_for(std::chrono::seconds(2));

  statusbar_log::RestoreCursorPosition();
  statusbar_log::ClearToEndOfLine();
  std::cout << "Clean message" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  statusbar_log::LogInf(kFilename, "Starting test...");
  std::cout << "\n\n";

  int err = 0;
  const int total_steps1 = 15;
  const int total_steps2 = 100;

  statusbar_log::StatusbarHandle h;

  err = statusbar_log::CreateStatusbarHandle(
      h, {2, 1},                                               // <-- Postions
      {20, 10},                                                // <-- Bar widths
      {"first:  ", "second: "},                                // <-- prefixes
      {" -- 15 total steps", "           -- 100 total steps"}  // <-- postfixes
  );
  if (err != 0) {
    statusbar_log::LogErr(kFilename, "Failed to create statusbar. Errorcode %d", err);
    return err;
  }

  for (std::size_t i = 0; i <= total_steps1; ++i) {
    double percent = static_cast<double>(i) / total_steps1 * 100.0;
    statusbar_log::UpdateStatusbar(h, 0, percent);
    if (i % 10 == 0 && i != 0) {
      statusbar_log::LogInf(kFilename, "10 Ticks reached");
    }
    for (std::size_t j = 0; j <= total_steps2; ++j) {
      double percent = static_cast<double>(j) / total_steps2 * 100.0;
      statusbar_log::UpdateStatusbar(h, 1, percent);
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
  }

  err = statusbar_log::DestroyStatusbarHandle(h);
  if (err != 0) {
    statusbar_log::LogErr(kFilename, "Failed to destroy statusbar. Errorcode %d", err);
    return err;
  }

  return 0;
}
