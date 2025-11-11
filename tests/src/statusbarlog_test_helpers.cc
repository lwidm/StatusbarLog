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

// clang-format off

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

#include "statusbarlog/statusbarlog.h"
#include "statusbarlog_test.h"

namespace statusbar_log {
namespace test {

void SetupTestOutputDirectory() {
  if (kSeparateLogFiles) {
    std::filesystem::remove_all(test_output_dir);
    if (!std::filesystem::create_directory(test_output_dir)){
    std::cerr << "Failed to create test output directory: " << test_output_dir
              << std::endl;
    }
  }
  if (!std::filesystem::remove_all(global_log_filename)){
    std::cerr << "Failed to create test log file: " << global_log_filename
              << std::endl;
}
}

std::string GenerateTestLogFilename(const std::string& test_suite,
                                    const std::string& test_name) {
  if (kSeparateLogFiles) {
    std::string safe_suite = test_suite;
    std::string safe_name = test_name;
    std::replace(safe_suite.begin(), safe_suite.end(), '/', '_');
    std::replace(safe_name.begin(), safe_name.end(), '/', '_');
    std::replace(safe_suite.begin(), safe_suite.end(), '\\', '_');
    std::replace(safe_name.begin(), safe_name.end(), '\\', '_');
    return test_output_dir + "/" + safe_suite + "_" + safe_name + ".log";
  }
  std::string safe_file_name = global_log_filename;
  std::replace(safe_file_name.begin(), safe_file_name.end(), '/', '_');
  std::replace(safe_file_name.begin(), safe_file_name.end(), '\\', '_');
  return safe_file_name;
}

int _CaptureStdoutToFile(const std::string& filename) {
  if (++_is_capturing > 1) {
    std::cerr << "CaptureStdoutToFile - Error: Already capturing stdout!\n";
    _is_capturing--;
    return -1;
  }

  std::fflush(stdout);
  std::cout.flush();

  int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0644);
  if (fd == -1) {
    std::cerr << "CaptureStdoutToFile - Error: open('" << filename
              << "') failed: " << std::strerror(errno) << "\n";
    _is_capturing--;
    return -2;
  }

  _saved_stdout_fd = dup(STDOUT_FILENO);
  if (_saved_stdout_fd == -1) {
    std::cerr << "CaptureStdoutToFile - Error: dup(STDOUT_FILENO) failed: "
              << std::strerror(errno) << "\n";
    close(fd);
    _is_capturing--;
    return -3;
  }

  if (dup2(fd, STDOUT_FILENO) == -1) {
    std::cerr << "CaptureStdoutToFile - Error: dup2(fd, STDOUT_FILENO) failed: "
              << std::strerror(errno) << "\n";
    close(fd);
    close(_saved_stdout_fd);
    _saved_stdout_fd = -1;
    _is_capturing--;
    return -4;
  }

  close(fd);
  std::ios::sync_with_stdio(true);
  std::fflush(stdout);
  std::cout.flush();
  return 0;
}

int _RestoreCaptureStdout() {
  if (_is_capturing == 0) {
    std::cerr << "RestoreCaptureStdout - Error: Not capturing stdout!\n";
    return -1;
  }

  std::fflush(stdout);
  std::cout.flush();

  if (_saved_stdout_fd == -1) {
    std::cerr << "RestoreCaptureStdout - Error: Saved fd invalid\n";
    return -2;
  }

  if (dup2(_saved_stdout_fd, STDOUT_FILENO) == -1) {
    std::cerr << "RestoreCaptureStdout - Error: dup2(_saved_stdout_fd, STDOUT_FILENO) "
                 "failed: "
              << std::strerror(errno) << "\n";
    close(_saved_stdout_fd);
    _saved_stdout_fd = -1;
    _is_capturing--;
    return -3;
  }

  close(_saved_stdout_fd);
  _saved_stdout_fd = -1;
  _is_capturing--;

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
  _CaptureStdoutToFile(log_filename);
  int err_code = statusbar_log::CreateStatusbarHandle(
      statusbar_handle, _positions, _bar_sizes, _prefixes, _postfixes);
  _RestoreCaptureStdout();
  return err_code;
}

int RedirectDestroyStatusbarHandle(
    statusbar_log::StatusbarHandle& statusbar_handle,
    const std::string& log_filename) {
  _CaptureStdoutToFile(log_filename);
  int err_code = statusbar_log::DestroyStatusbarHandle(statusbar_handle);
  _RestoreCaptureStdout();
  return err_code;
}

int RedirectUpdateStatusbar(statusbar_log::StatusbarHandle& statusbar_handle,
                            const std::size_t idx, const double percent,
                            const std::string& log_filename) {
  _CaptureStdoutToFile(log_filename);
  int err_code = statusbar_log::UpdateStatusbar(statusbar_handle, idx, percent);
  _RestoreCaptureStdout();
  return err_code;
}

}  // namespace test
}  // namespace statusbar_log
