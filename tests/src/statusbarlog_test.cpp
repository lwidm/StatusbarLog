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

// -- statusbarlog/test/src/statusbarlog_test.cpp

// clang-format off

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <algorithm>
#include <filesystem>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

#include "statusbarlog/statusbarlog.h"

// clang-format on

static int _saved_stdout_fd = -1;
static bool _is_capturing = false;
static std::string test_output_dir = "test_output";

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

  int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0644);
  if (fd == -1) {
    std::cerr << "CaptureStdoutToFile - Error: open('" << filename
              << "') failed: " << std::strerror(errno) << "\n";
    return -2;
  }

  _saved_stdout_fd = dup(STDOUT_FILENO);
  if (_saved_stdout_fd == -1) {
    std::cerr << "CaptureStdoutToFile - Error: dup(STDOUT_FILENO) failed: "
              << std::strerror(errno) << "\n";
    close(fd);
    return -3;
  }

  if (dup2(fd, STDOUT_FILENO) == -1) {
    std::cerr << "CaptureStdoutToFile - Error: dup2(fd, STDOUT_FILENO) failed: "
              << std::strerror(errno) << "\n";
    close(fd);
    close(_saved_stdout_fd);
    _saved_stdout_fd = -1;
    return -4;
  }

  close(fd);
  std::ios::sync_with_stdio(true);
  std::fflush(stdout);
  std::cout.flush();

  _is_capturing = true;
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
  // CaptureStdoutToFile(log_filename);
  int err_code = statusbar_log::UpdateStatusbar(statusbar_handle, idx, percent);
  // RestoreStdout();
  return err_code;
}

// ==================================================
// Base Test Fixture with Auto Log Filename Generation
// ==================================================

class StatusbarTestBase : public ::testing::Test {
 protected:
  std::string GetTestLogFilename() {
    const auto* test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();
    return GenerateTestLogFilename(test_info->test_suite_name(),
                                   test_info->name());
  }
};

// ==================================================
// HandleManagementTest
// ==================================================

class HandleManagementTest : public StatusbarTestBase {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(HandleManagementTest, CreateSingleBarHandle) {
  statusbar_log::StatusbarHandle handle;
  std::vector<unsigned int> positions = {1};
  std::vector<unsigned int> bar_sizes = {50};
  std::vector<std::string> prefixes = {"Processing"};
  std::vector<std::string> postfixes = {"items"};

  const std::string log_filename = GetTestLogFilename();
  int err_code = RedirectCreateStatusbarHandle(
      handle, positions, bar_sizes, prefixes, postfixes, log_filename);
  ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "CreateStatusbarHandle should return "
         "statusbar_log::kStatusbarLogSuccess";
  EXPECT_TRUE(handle.valid)
      << "Handle should be marked as valid after creation";
  EXPECT_NE(handle.id, 0) << "Handle should have a non-zero ID assigned";
  EXPECT_LT(handle.id, SIZE_MAX) << "Handle index should be a valid value";
  EXPECT_LT(handle.idx, statusbar_log::kMaxStatusbarHandles)
      << "Handle index should be less than statusbar_log::kMaxStatusbarHandles";

  err_code = RedirectUpdateStatusbar(handle, 0, 50.0, log_filename);
  EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should be able to update the status bar with valid handle";

  err_code = RedirectDestroyStatusbarHandle(handle, log_filename);
  EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle.valid)
      << "Handle should be marked as invalid after destruction";
}

TEST_F(HandleManagementTest, CreateMultiBarHandle) {
  statusbar_log::StatusbarHandle handle;
  std::vector<unsigned int> positions = {2, 1};
  std::vector<unsigned int> bar_sizes = {20, 10};
  std::vector<std::string> prefixes = {"first", "second"};
  std::vector<std::string> postfixes = {"20 long", "10 long"};

  const std::string log_filename = GetTestLogFilename();
  int err_code = RedirectCreateStatusbarHandle(
      handle, positions, bar_sizes, prefixes, postfixes, log_filename);

  EXPECT_NE(handle.id, 0) << "Handle should have a non-zero ID assigned";
  EXPECT_LT(handle.id, SIZE_MAX) << "Handle index should be a valid value";
  EXPECT_LT(handle.idx, statusbar_log::kMaxStatusbarHandles)
      << "Handle index should be less than statusbar_log::kMaxStatusbarHandles";
  err_code = RedirectUpdateStatusbar(handle, 0, 50.0, log_filename);
  EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should be able to update the status bar with valid handle";
  err_code = RedirectDestroyStatusbarHandle(handle, log_filename);
  EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle.valid)
      << "Handle should be marked as invalid after destruction";
}

TEST_F(HandleManagementTest, CreateHandle_InvalidInputSizes) {
  statusbar_log::StatusbarHandle handle;

  // Test Case 1: Positions vector larger than others
  {
    std::vector<unsigned int> positions = {1, 2};
    std::vector<unsigned int> bar_sizes = {50};
    std::vector<std::string> prefixes = {"Processing"};
    std::vector<std::string> postfixes = {"items"};

    const std::string log_filename = GetTestLogFilename();
    int err_code = RedirectCreateStatusbarHandle(
        handle, positions, bar_sizes, prefixes, postfixes, log_filename);

    EXPECT_NE(err_code, statusbar_log::kStatusbarLogSuccess)
        << "Should fail when positions vector (size " << positions.size()
        << ") is larger than others (sizes " << bar_sizes.size() << ","
        << prefixes.size() << "," << postfixes.size() << ")";

    EXPECT_FALSE(handle.valid)
        << "Handle should remain invalid after failed creation";
  }

  // Test Case 2: Bar sizes vector larger than others
  {
    std::vector<unsigned int> positions = {1};
    std::vector<unsigned int> bar_sizes = {50, 70};
    std::vector<std::string> prefixes = {"Processing"};
    std::vector<std::string> postfixes = {"items"};

    const std::string log_filename = GetTestLogFilename();
    int err_code = RedirectCreateStatusbarHandle(
        handle, positions, bar_sizes, prefixes, postfixes, log_filename);

    EXPECT_NE(err_code, statusbar_log::kStatusbarLogSuccess)
        << "Should fail when bar_sizes vector (size " << bar_sizes.size()
        << ") is larger than others (sizes " << positions.size() << ","
        << prefixes.size() << "," << postfixes.size() << ")";

    EXPECT_FALSE(handle.valid)
        << "Handle should remain invalid after failed creation";
  }

  // Test Case 3: prefixes vector larger than others
  {
    std::vector<unsigned int> positions = {1};
    std::vector<unsigned int> bar_sizes = {50};
    std::vector<std::string> prefixes = {"Processing", "more"};
    std::vector<std::string> postfixes = {"items"};

    const std::string log_filename = GetTestLogFilename();
    int err_code = RedirectCreateStatusbarHandle(
        handle, positions, bar_sizes, prefixes, postfixes, log_filename);

    EXPECT_NE(err_code, statusbar_log::kStatusbarLogSuccess)
        << "Should fail when prefixes vector (size " << prefixes.size()
        << ") is larger than others (sizes " << positions.size() << ","
        << bar_sizes.size() << "," << postfixes.size() << ")";

    EXPECT_FALSE(handle.valid)
        << "Handle should remain invalid after failed creation";
  }

  // Test Case 4: postfixes vector larger than others
  {
    std::vector<unsigned int> positions = {1};
    std::vector<unsigned int> bar_sizes = {50};
    std::vector<std::string> prefixes = {"Processing"};
    std::vector<std::string> postfixes = {"items", "more"};

    const std::string log_filename = GetTestLogFilename();
    int err_code = RedirectCreateStatusbarHandle(
        handle, positions, bar_sizes, prefixes, postfixes, log_filename);

    EXPECT_NE(err_code, statusbar_log::kStatusbarLogSuccess)
        << "Should fail when postfixes vector (size " << postfixes.size()
        << ") is larger than others (sizes " << positions.size() << ","
        << bar_sizes.size() << "," << prefixes.size() << ")";

    EXPECT_FALSE(handle.valid)
        << "Handle should remain invalid after failed creation";
  }
}

TEST_F(HandleManagementTest, CreateHandle_MaxActiveHandlesLimit) {
  std::vector<statusbar_log::StatusbarHandle> handles;
  bool reached_limit = false;

  for (size_t i = 0; i < statusbar_log::kMaxStatusbarHandles + 5; ++i) {
    statusbar_log::StatusbarHandle handle;
    std::vector<unsigned int> positions = {1};
    std::vector<unsigned int> bar_sizes = {50};
    std::vector<std::string> prefixes = {"Test " + std::to_string(i)};
    std::vector<std::string> postfixes = {"item"};

    const std::string log_filename = GetTestLogFilename();
    int err_code = RedirectCreateStatusbarHandle(
        handle, positions, bar_sizes, prefixes, postfixes, log_filename);

    if (err_code == statusbar_log::kStatusbarLogSuccess) {
      // Successfully created - should only happen up to
      // statusbar_log::kMaxStatusbarHandles
      EXPECT_LE(handles.size(), statusbar_log::kMaxStatusbarHandles - 1)
          << "Should not create more than "
          << statusbar_log::kMaxStatusbarHandles << " handles";
      EXPECT_TRUE(handle.valid);
      handles.push_back(handle);
    } else {
      // Should fail with specific error code
      EXPECT_EQ(err_code, -3)
          << "Should return error code -3 when max handles limit reached";
      EXPECT_FALSE(handle.valid)
          << "Handle should be invalid when creation fails";
      reached_limit = true;

      break;
    }
  }

  EXPECT_TRUE(reached_limit)
      << "Should have encountered the maximum handles limit";
  EXPECT_EQ(handles.size(), statusbar_log::kMaxStatusbarHandles)
      << "Should have created exactly " << statusbar_log::kMaxStatusbarHandles
      << " handles";

  // Test that we can still destroy handles and create new ones after cleanup
  if (!handles.empty()) {
    // Destroy one handle
    const std::string log_filename = GetTestLogFilename();
    int err_code = RedirectDestroyStatusbarHandle(handles[0], log_filename);
    EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess);
    handles.erase(handles.begin());

    // Now we should be able to create one more handle
    statusbar_log::StatusbarHandle new_handle;
    std::vector<unsigned int> positions = {1};
    std::vector<unsigned int> bar_sizes = {50};
    std::vector<std::string> prefixes = {"New after destroy"};
    std::vector<std::string> postfixes = {"works"};

    err_code = RedirectCreateStatusbarHandle(new_handle, positions, bar_sizes,
                                             prefixes, postfixes, log_filename);

    EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
        << "Should be able to create new handle after destroying one";
    EXPECT_TRUE(new_handle.valid);
    handles.push_back(new_handle);
  }

  for (auto& handle : handles) {
    const std::string log_filename = GetTestLogFilename();
    RedirectDestroyStatusbarHandle(handle, log_filename);
  }
}

TEST_F(HandleManagementTest, DestroyValidHandle) {
  statusbar_log::StatusbarHandle handle;
  std::vector<unsigned int> positions = {1};
  std::vector<unsigned int> bar_sizes = {50};
  std::vector<std::string> prefixes = {"Processing"};
  std::vector<std::string> postfixes = {"items"};

  const std::string log_filename = GetTestLogFilename();
  int err_code = RedirectCreateStatusbarHandle(
      handle, positions, bar_sizes, prefixes, postfixes, log_filename);

  EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "CreateStatusbarHandle should return "
         "statusbar_log::kStatusbarLogSuccess";

  err_code = RedirectDestroyStatusbarHandle(handle, log_filename);

  EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle.valid)
      << "Handle should be marked as invalid after destruction";
}

TEST_F(HandleManagementTest, DestroyInvalidHandle) {
  statusbar_log::StatusbarHandle handle;
  std::vector<unsigned int> positions = {1};
  std::vector<unsigned int> bar_sizes = {50};
  std::vector<std::string> prefixes = {"Processing"};
  std::vector<std::string> postfixes = {"items"};

  const std::string log_filename = GetTestLogFilename();
  int err_code = RedirectDestroyStatusbarHandle(handle, log_filename);

  EXPECT_NE(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should not be able to destroy a statusbar before it has been created";

  err_code = RedirectCreateStatusbarHandle(handle, positions, bar_sizes,
                                           prefixes, postfixes, log_filename);
  ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "CreateStatusbarHandle should return "
         "statusbar_log::kStatusbarLogSuccess";

  std::size_t tmp_idx = handle.idx;
  handle.idx = SIZE_MAX;

  err_code = RedirectDestroyStatusbarHandle(handle, log_filename);

  EXPECT_NE(err_code, statusbar_log::kStatusbarLogSuccess)
      << "If indexes don't match, destroying statusbars should not be possible";

  handle.idx = tmp_idx;
  err_code = RedirectDestroyStatusbarHandle(handle, log_filename);

  EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle.valid)
      << "Handle should be marked as invalid after destruction";
}

TEST_F(HandleManagementTest, DestroyAlreadyDestroyedHandle) {
  statusbar_log::StatusbarHandle handle;
  std::vector<unsigned int> positions = {1};
  std::vector<unsigned int> bar_sizes = {50};
  std::vector<std::string> prefixes = {"Processing"};
  std::vector<std::string> postfixes = {"items"};

  const std::string log_filename = GetTestLogFilename();
  int err_code = RedirectCreateStatusbarHandle(
      handle, positions, bar_sizes, prefixes, postfixes, log_filename);
  ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "CreateStatusbarHandle should return "
         "statusbar_log::kStatusbarLogSuccess";

  err_code = RedirectDestroyStatusbarHandle(handle, log_filename);

  ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle.valid)
      << "Handle should be marked as invalid after destruction";

  err_code = RedirectDestroyStatusbarHandle(handle, log_filename);

  EXPECT_NE(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should not be able to destroy already destroyed handle";
}

// ==================================================
// StatusbarUpdateTest
// ==================================================

class StatusbarUpdateTest : public StatusbarTestBase {
 protected:
  std::string log_filename;
  statusbar_log::StatusbarHandle handle;
  std::vector<unsigned int> positions;
  std::vector<unsigned int> bar_sizes;
  std::vector<std::string> prefixes;
  std::vector<std::string> postfixes;

  void SetUp() override {
    this->positions = {1};
    this->bar_sizes = {50};
    this->prefixes = {"Processing"};
    this->postfixes = {"items"};
    this->log_filename = GetTestLogFilename();

    int err_code = RedirectCreateStatusbarHandle(
        this->handle, this->positions, this->bar_sizes, this->prefixes,
        this->postfixes, this->log_filename);

    ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
        << "Failed to create statusbar handle in fixture setup";
    ASSERT_TRUE(handle.valid)
        << "Handle should be valid after creation in fixture setup";
  }

  void TearDown() override {
    // Only destroy if the handle is still valid
    if (handle.valid) {
      int err_code = RedirectDestroyStatusbarHandle(handle, log_filename);
      EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
          << "Failed to destroy statusbar handle in fixture teardown";
    }
  }

  void UpdateBarAndVerify(size_t bar_index, double percentage) {
    int err_code = RedirectUpdateStatusbar(this->handle, bar_index, percentage,
                                           log_filename);
    EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
        << "Failed to update bar at index " << bar_index << " with percentage "
        << percentage;
  }
  void updateBarAndVerifyFailure(size_t bar_index, double percentage) {
    int err_code = RedirectUpdateStatusbar(this->handle, bar_index, percentage,
                                           log_filename);
    EXPECT_NE(err_code, statusbar_log::kStatusbarLogSuccess)
        << "Should have failed to update bar with index " << bar_index
        << " with percentage " << percentage;
  }
};

TEST_F(StatusbarUpdateTest, UpdateValidPercentage) {
  UpdateBarAndVerify(0, 0.0);
  UpdateBarAndVerify(0, 25.5);
  UpdateBarAndVerify(0, 50.0);
  UpdateBarAndVerify(0, 75.0);
  UpdateBarAndVerify(0, 100.0);
}

TEST_F(StatusbarUpdateTest, UpdateBoundaryPercentages) {
  UpdateBarAndVerify(0, 0.0);
  UpdateBarAndVerify(0, 0.1);
  UpdateBarAndVerify(0, 99.9);
  UpdateBarAndVerify(0, 100.0);
}

TEST_F(StatusbarUpdateTest, UpdateMultipleBarsInHandle) {
  int err = RedirectDestroyStatusbarHandle(this->handle, this->log_filename);
  ASSERT_EQ(err, statusbar_log::kStatusbarLogSuccess)
      << "Failed to destroy statusbar handle "
         "in UpdateMultipleBarsInHandle test";

  this->positions = {2, 1};
  this->bar_sizes = {50, 25};
  this->prefixes = {"second", "first"};
  this->postfixes = {"items", "things"};

  int err_code = RedirectCreateStatusbarHandle(
      this->handle, this->positions, this->bar_sizes, this->prefixes,
      this->postfixes, this->log_filename);

  ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Failed to create multiple statusbars in UpdateMultipleBarsInHandle "
         "test";

  UpdateBarAndVerify(0, 0.0);
  UpdateBarAndVerify(1, 0.0);
  UpdateBarAndVerify(0, 25.5);
  UpdateBarAndVerify(0, 50.0);
  UpdateBarAndVerify(1, 50.0);
  UpdateBarAndVerify(0, 75.0);
  UpdateBarAndVerify(1, 75.0);
  UpdateBarAndVerify(0, 100.0);
  UpdateBarAndVerify(1, 100.0);
}

TEST_F(StatusbarUpdateTest, InvalidUpdates) {
  updateBarAndVerifyFailure(0, -1.0);
  updateBarAndVerifyFailure(0, 101.0);
  updateBarAndVerifyFailure(1, 100.0);

  statusbar_log::StatusbarHandle handle2;
  std::vector<unsigned int> positions2 = {1};
  std::vector<unsigned int> bar_sizes2 = {50};
  std::vector<std::string> prefixes2 = {"Processing"};
  std::vector<std::string> postfixes2 = {"items"};

  const std::string log_filename2 = GetTestLogFilename();
  int err_code = RedirectCreateStatusbarHandle(
      handle2, positions2, bar_sizes2, prefixes2, postfixes2, log_filename2);
  ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "CreateStatusbarHandle should return "
         "statusbar_log::kStatusbarLogSuccess";

  err_code = RedirectDestroyStatusbarHandle(handle2, log_filename2);

  ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle2.valid)
      << "Handle should be marked as invalid after destruction";

  err_code = RedirectUpdateStatusbar(handle2, 0, 20, log_filename2);

  EXPECT_NE(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Should not be able to destroy already destroyed handle";
}

// ==================================================
// StatusbarValidations
// ==================================================

class StatusbarValidations : public StatusbarTestBase {
 protected:
  std::string log_filename;
  statusbar_log::StatusbarHandle handle;
  std::vector<unsigned int> positions;
  std::vector<unsigned int> bar_sizes;
  std::vector<std::string> prefixes;
  std::vector<std::string> postfixes;

  void SetUp() override {
    this->positions = {1};
    this->bar_sizes = {50};
    this->prefixes = {"Processing"};
    this->postfixes = {"items"};
    this->log_filename = GetTestLogFilename();

    int err_code = RedirectCreateStatusbarHandle(
        this->handle, this->positions, this->bar_sizes, this->prefixes,
        this->postfixes, this->log_filename);

    ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
        << "Failed to create statusbar handle in fixture setup";
    ASSERT_TRUE(handle.valid)
        << "Handle should be valid after creation in fixture setup";
  }
};

TEST_F(StatusbarValidations, IsValidHandle) {
  statusbar_log::StatusbarHandle handle2;
  std::vector<unsigned int> positions = {1};
  std::vector<unsigned int> bar_sizes = {50};
  std::vector<std::string> prefixes = {"Processing"};
  std::vector<std::string> postfixes = {"items"};

  std::string log_filename2 = GetTestLogFilename();
  int err_code = RedirectCreateStatusbarHandle(
      handle2, positions, bar_sizes, prefixes, postfixes, log_filename2);
  ASSERT_EQ(err_code, statusbar_log::kStatusbarLogSuccess)
      << "Failed to create statusbar handle";
  ASSERT_TRUE(handle2.valid) << "Handle should be valid after creation";

  handle.valid = false;
  err_code = RedirectDestroyStatusbarHandle(handle, this->log_filename);
  EXPECT_EQ(err_code, -1);

  handle.valid = true;
  int idx_backup = handle.idx;
  handle.idx = SIZE_MAX;
  err_code = RedirectDestroyStatusbarHandle(handle, this->log_filename);
  EXPECT_EQ(err_code, -2);

  handle.idx = 99999;
  err_code = RedirectDestroyStatusbarHandle(handle, this->log_filename);
  EXPECT_EQ(err_code, -2);

  handle.idx = idx_backup + 1;
  err_code = RedirectDestroyStatusbarHandle(handle, this->log_filename);
  EXPECT_EQ(err_code, -3);

  handle.idx = idx_backup;
  int ID_backup = handle.id;
  handle.id = handle2.id;
  err_code = RedirectDestroyStatusbarHandle(handle, this->log_filename);
  EXPECT_EQ(err_code, -3);

  handle.id = ID_backup + 1;
  err_code = RedirectDestroyStatusbarHandle(handle, this->log_filename);
  EXPECT_EQ(err_code, -3);

  // BUG : Cant test err_code -4

  handle.id = ID_backup;
  err_code = RedirectDestroyStatusbarHandle(handle, this->log_filename);
  EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess);
  err_code = RedirectDestroyStatusbarHandle(handle2, log_filename2);
  EXPECT_EQ(err_code, statusbar_log::kStatusbarLogSuccess);
}

// ==================================================
// Main function with test directory management
// ==================================================

int main(int argc, char** argv) {
  // Set up the test output directory
  if (!SetupTestOutputDirectory()) {
    std::cerr << "Failed to create test output directory: " << test_output_dir
              << std::endl;
    return 1;
  }

  std::cout << "Test output directory created: " << test_output_dir
            << std::endl;

  ::testing::InitGoogleTest(&argc, argv);

  int result = RUN_ALL_TESTS();

  std::cout << "Test logs available in: " << test_output_dir << std::endl;

  return result;
}
