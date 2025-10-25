// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 Lukas Widmer
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// -- statusbarlog/test/src/statusbarlog_main.cpp

// clang-format off

#include <cstddef>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "statusbarlog/statusbarlog.h"

// clang-format on

const std::string kFilename = "statusbarlog_main.cpp";

void PrintWithCleanup() {
  std::cout << "Start to be kept <- " << std::flush;
  ;
  statusbar_log::SaveCursorPosition();
  std::cout << "Temporary message that might be long" << std::flush;
  ;
  std::this_thread::sleep_for(std::chrono::seconds(2));

  statusbar_log::RestoreCursorPosition();
  statusbar_log::ClearToEndOfLine();
  std::cout << "Clean message" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
  PrintWithCleanup();
  statusbar_log::LogInf(kFilename, "Starting test...");
  statusbar_log::LogInf(kFilename, "Starting test...");
  statusbar_log::LogInf(kFilename, "Starting test...");
  statusbar_log::LogInf
 (kFilename, "Starting test...");
  // statusbar_log::LogInf("ExremelyLongFilenameWhichShouldBeTruncated", "Lorem ipsum dolor
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
      statusbar_log::LogInf
     (kFilename, "10 Ticks reached");
    }
    if (i % 3 == 0 && i != 0) {
      // statusbar_log::UpdateStatusbar(h, 0, 100.1);
      // statusbar_log::UpdateStatusbar(h, 0, -0.1);
    }

    // Simulate work:
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
