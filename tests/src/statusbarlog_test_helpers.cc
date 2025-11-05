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

// -- statusbarlog/test/src/statusbarlog_test_helpers.h

#include "statusbarlog_test.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <algorithm>
#include <filesystem>
#include <cstring>
#include <iostream>
#include <string>

#include "statusbarlog/statusbarlog.h"


namespace statusbar_log {
namespace test{

bool SetupTestOutputDirectory() {
  std::filesystem::remove_all(test_output_dir);
  return std::filesystem::create_directory(test_output_dir);
}

std::string GenerateTestLogFilename(const std::string& test_suite,
                                    const std::string& test_name) {
  std::string safe_suite = test_suite;
  std::string safe_name = test_name;
  std::replace(safe_suite.begin(), safe_suite.end(), '/', '_');
  std::replace(safe_name.begin(), safe_name.end(), '/', '_');
  return test_output_dir + "/" + safe_suite + "_" + safe_name + ".log";
}

int CaptureStdoutToFile(const std::string& filename) {
  if (_is_capturing) {
    std::cerr << "CaptureStdoutToFile - Error: Already capturing stdout!\n";
    return -1;
  }

  std::fflush(stdout);
  std::cout.flush();

  // int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0644);
  // if (fd == -1) {
  //   std::cerr << "CaptureStdoutToFile - Error: open('" << filename
  //             << "') failed: " << std::strerror(errno) << "\n";
  //   return -2;
  // }


  _saved_stdout_fd = dup(STDOUT_FILENO);
  if (_saved_stdout_fd == -1) {
    std::cerr << "CaptureStdoutToFile - Error: dup(STDOUT_FILENO) failed: "
              << std::strerror(errno) << "\n";
    // close(fd);
    return -2;
  }

  FILE* f = std::freopen(filename.c_str(), "w", stdout);
  if (f == nullptr) {
    std::cerr << "CaptureStdoutToFile - Error: freopen('" << filename
              << "') failed: " << std::strerror(errno) << "\n";
    close(_saved_stdout_fd);
    _saved_stdout_fd = -1;
    return -3;
  }
  setvbuf(stdout, nullptr, _IOFBF, 0);

  // if (dup2(fd, STDOUT_FILENO) == -1) {
  //   std::cerr << "CaptureStdoutToFile - Error: dup2(fd, STDOUT_FILENO) failed: "
  //             << std::strerror(errno) << "\n";
  //   close(fd);
  //   close(_saved_stdout_fd);
  //   _saved_stdout_fd = -1;
  //   return -4;
  // }

  // close(fd);
  std::ios::sync_with_stdio(true);
  std::fflush(stdout);
  std::cout.flush();

  _is_capturing = true;
  std::cout << "HERE 6\n" << std::endl;
  return 0;
}

int RestoreStdout() {
  if (!_is_capturing) {
    std::cerr << "RestoreStdout - Error: Not capturing stdout!\n";
    return -1;
  }

  std::fflush(stdout);
  std::cout.flush();

  if (_saved_stdout_fd == -1) {
    std::cerr << "RestoreStdout - Error: Saved fd invalid\n";
    return -2;
  }

  if (dup2(_saved_stdout_fd, STDOUT_FILENO) == -1) {
    std::cerr << "RestoreStdout - Error: dup2(_saved_stdout_fd, STDOUT_FILENO) "
                 "failed: "
              << std::strerror(errno) << "\n";
    close(_saved_stdout_fd);
    _saved_stdout_fd = -1;
    _is_capturing = false;
    return -3;
  }

  close(_saved_stdout_fd);
  _saved_stdout_fd = -1;
  _is_capturing = false;

  std::fflush(stdout);
  std::cout.flush();
  std::ios::sync_with_stdio(true);

  return 0;
}

int RedirectCreateStatusbarHandle(
    statusbar_log::StatusbarHandle& statusbar_handle,
    const std::vector<unsigned int> _positions,
    const std::vector<unsigned int> _bar_sizes,
    const std::vector<std::string> _prefixes,
    const std::vector<std::string> _postfixes,
    const std::string& log_filename) {
  CaptureStdoutToFile(log_filename);
  int err_code = statusbar_log::CreateStatusbarHandle(
      statusbar_handle, _positions, _bar_sizes, _prefixes, _postfixes);
  RestoreStdout();
  return err_code;
}

int RedirectDestroyStatusbarHandle(
    statusbar_log::StatusbarHandle& statusbar_handle,
    const std::string& log_filename) {
  CaptureStdoutToFile(log_filename);
  int err_code = statusbar_log::DestroyStatusbarHandle(statusbar_handle);
  RestoreStdout();
  return err_code;
}

int RedirectUpdateStatusbar(statusbar_log::StatusbarHandle& statusbar_handle,
                            const std::size_t idx, const double percent,
                            const std::string& log_filename) {

  std::cout << "starting update\n" << std::endl;
  std::cout << "HERE 5\n" << std::endl;
  CaptureStdoutToFile(log_filename);
  std::cout << "HERE 7\n" << std::endl;
  std::cout << "\n\n\n";
  std::cout.flush();
  int err_code = statusbar_log::UpdateStatusbar(statusbar_handle, idx, percent);
  std::cout << "HERE 8\n" << std::endl;
  RestoreStdout();
  std::cout << "HERE 9\n" << std::endl;
  return err_code;
}

} // statusbar_log
} // test
