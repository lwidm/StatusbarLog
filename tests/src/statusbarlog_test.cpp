// -- statusbarlog/test/src/statusbarlog_test.cpp
#include "statusbarlog/statusbarlog.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

// ==================================================
// HandleManagementTest
// ==================================================

class HandleManagementTest : public ::testing::Test {
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

  int err_code = statusbar_log::create_statusbar_handle(
      handle, positions, bar_sizes, prefixes, postfixes);

  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "create_statusbar_handle should return STATUSBARLOG_SUCCESS";
  EXPECT_TRUE(handle.valid)
      << "Handle should be marked as valid after creation";
  EXPECT_NE(handle.id, 0) << "Handle should have a non-zero ID assigned";
  EXPECT_LT(handle.id, SIZE_MAX) << "Handle index should be a valid value";
  EXPECT_LT(handle.idx, MAX_HANDLES)
      << "Handle index should be less than MAX_HANDLES";
  EXPECT_EQ(statusbar_log::update_statusbar(handle, 0, 50.0),
            STATUSBARLOG_SUCCESS)
      << "Should be able to update the status bar with valid handle";
  EXPECT_EQ(statusbar_log::destroy_statusbar_handle(handle),
            STATUSBARLOG_SUCCESS)
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

  int err_code = statusbar_log::create_statusbar_handle(
      handle, positions, bar_sizes, prefixes, postfixes);

  EXPECT_NE(handle.id, 0) << "Handle should have a non-zero ID assigned";
  EXPECT_LT(handle.id, SIZE_MAX) << "Handle index should be a valid value";
  EXPECT_LT(handle.idx, MAX_HANDLES)
      << "Handle index should be less than MAX_HANDLES";
  EXPECT_EQ(statusbar_log::update_statusbar(handle, 0, 50.0),
            STATUSBARLOG_SUCCESS)
      << "Should be able to update the status bar with valid handle";
  EXPECT_EQ(statusbar_log::destroy_statusbar_handle(handle),
            STATUSBARLOG_SUCCESS)
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

    int err_code = statusbar_log::create_statusbar_handle(
        handle, positions, bar_sizes, prefixes, postfixes);

    EXPECT_NE(err_code, STATUSBARLOG_SUCCESS)
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

    int err_code = statusbar_log::create_statusbar_handle(
        handle, positions, bar_sizes, prefixes, postfixes);

    EXPECT_NE(err_code, STATUSBARLOG_SUCCESS)
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

    int err_code = statusbar_log::create_statusbar_handle(
        handle, positions, bar_sizes, prefixes, postfixes);

    EXPECT_NE(err_code, STATUSBARLOG_SUCCESS)
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

    int err_code = statusbar_log::create_statusbar_handle(
        handle, positions, bar_sizes, prefixes, postfixes);

    EXPECT_NE(err_code, STATUSBARLOG_SUCCESS)
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

  for (size_t i = 0; i < MAX_HANDLES + 5; ++i) {
    statusbar_log::StatusbarHandle handle;
    std::vector<unsigned int> positions = {1};
    std::vector<unsigned int> bar_sizes = {50};
    std::vector<std::string> prefixes = {"Test " + std::to_string(i)};
    std::vector<std::string> postfixes = {"item"};

    int err_code = statusbar_log::create_statusbar_handle(
        handle, positions, bar_sizes, prefixes, postfixes);

    if (err_code == STATUSBARLOG_SUCCESS) {
      // Successfully created - should only happen up to MAX_HANDLES
      EXPECT_LE(handles.size(), MAX_HANDLES - 1)
          << "Should not create more than " << MAX_HANDLES << " handles";
      EXPECT_TRUE(handle.valid);
      handles.push_back(handle);
    } else {
      // Should fail with specific error code
      EXPECT_EQ(err_code, -2)
          << "Should return error code -2 when max handles limit reached";
      EXPECT_FALSE(handle.valid)
          << "Handle should be invalid when creation fails";
      reached_limit = true;

      break;
    }
  }

  EXPECT_TRUE(reached_limit)
      << "Should have encountered the maximum handles limit";
  EXPECT_EQ(handles.size(), MAX_HANDLES)
      << "Should have created exactly " << MAX_HANDLES << " handles";

  // Test that we can still destroy handles and create new ones after cleanup
  if (!handles.empty()) {
    // Destroy one handle
    int err_code = statusbar_log::destroy_statusbar_handle(handles[0]);
    EXPECT_EQ(err_code, STATUSBARLOG_SUCCESS);
    handles.erase(handles.begin());

    // Now we should be able to create one more handle
    statusbar_log::StatusbarHandle new_handle;
    std::vector<unsigned int> positions = {1};
    std::vector<unsigned int> bar_sizes = {50};
    std::vector<std::string> prefixes = {"New after destroy"};
    std::vector<std::string> postfixes = {"works"};

    err_code = statusbar_log::create_statusbar_handle(
        new_handle, positions, bar_sizes, prefixes, postfixes);

    EXPECT_EQ(err_code, STATUSBARLOG_SUCCESS)
        << "Should be able to create new handle after destroying one";
    EXPECT_TRUE(new_handle.valid);
    handles.push_back(new_handle);
  }

  for (auto& handle : handles) {
    statusbar_log::destroy_statusbar_handle(handle);
  }
}

TEST_F(HandleManagementTest, DestroyValidHandle) {
  statusbar_log::StatusbarHandle handle;
  std::vector<unsigned int> positions = {1};
  std::vector<unsigned int> bar_sizes = {50};
  std::vector<std::string> prefixes = {"Processing"};
  std::vector<std::string> postfixes = {"items"};

  int err_code = statusbar_log::create_statusbar_handle(
      handle, positions, bar_sizes, prefixes, postfixes);

  EXPECT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "create_statusbar_handle should return STATUSBARLOG_SUCCESS";

  err_code = statusbar_log::destroy_statusbar_handle(handle);

  EXPECT_EQ(err_code, STATUSBARLOG_SUCCESS)
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

  int err_code = statusbar_log::destroy_statusbar_handle(handle);

  EXPECT_NE(err_code, STATUSBARLOG_SUCCESS)
      << "Should not be able to destroy a statusbar before it has been created";

  err_code = statusbar_log::create_statusbar_handle(handle, positions, bar_sizes,
                                                   prefixes, postfixes);
  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "create_statusbar_handle should return STATUSBARLOG_SUCCESS";

  std::size_t tmp_idx = handle.idx;
  handle.idx = SIZE_MAX;

  err_code = statusbar_log::destroy_statusbar_handle(handle);

  EXPECT_NE(err_code, STATUSBARLOG_SUCCESS)
      << "If indexes don't match, destroying statusbars should not be possible";

  handle.idx = tmp_idx;
  err_code = statusbar_log::destroy_statusbar_handle(handle);

  EXPECT_EQ(err_code, STATUSBARLOG_SUCCESS)
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

  int err_code = statusbar_log::create_statusbar_handle(
      handle, positions, bar_sizes, prefixes, postfixes);
  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "create_statusbar_handle should return STATUSBARLOG_SUCCESS";

  err_code = statusbar_log::destroy_statusbar_handle(handle);

  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle.valid)
      << "Handle should be marked as invalid after destruction";

  err_code = statusbar_log::destroy_statusbar_handle(handle);

  EXPECT_NE(err_code, STATUSBARLOG_SUCCESS)
      << "Should not be able to destroy already destroyed handle";
}

// ==================================================
// StatusbarUpdateTest
// ==================================================

class StatusbarUpdateTest : public ::testing::Test {
 protected:
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

    int err_code = statusbar_log::create_statusbar_handle(
        this->handle, this->positions, this->bar_sizes, this->prefixes,
        this->postfixes);

    ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
        << "Failed to create statusbar handle in fixture setup";
    ASSERT_TRUE(handle.valid)
        << "Handle should be valid after creation in fixture setup";
  }

  void TearDown() override {
    // Only destroy if the handle is still valid
    if (handle.valid) {
      int err_code = statusbar_log::destroy_statusbar_handle(handle);
      EXPECT_EQ(err_code, STATUSBARLOG_SUCCESS)
          << "Failed to destroy statusbar handle in fixture teardown";
    }
  }

  void updateBarAndVerify(size_t bar_index, double percentage) {
    int err_code =
        statusbar_log::update_statusbar(this->handle, bar_index, percentage);
    EXPECT_EQ(err_code, STATUSBARLOG_SUCCESS)
        << "Failed to update bar at index " << bar_index << " with percentage "
        << percentage;
  }
  void updateBarAndVerifyFailure(size_t bar_index, double percentage) {
    int err_code =
        statusbar_log::update_statusbar(this->handle, bar_index, percentage);
    EXPECT_NE(err_code, STATUSBARLOG_SUCCESS)
        << "Should have failed to update bar with index " << bar_index
        << " with percentage " << percentage;
  }
};

TEST_F(StatusbarUpdateTest, UpdateValidPercentage) {
  updateBarAndVerify(0, 0.0);
  updateBarAndVerify(0, 25.5);
  updateBarAndVerify(0, 50.0);
  updateBarAndVerify(0, 75.0);
  updateBarAndVerify(0, 100.0);
}

TEST_F(StatusbarUpdateTest, UpdateBoundaryPercentages) {
  updateBarAndVerify(0, 0.0);
  updateBarAndVerify(0, 0.1);
  updateBarAndVerify(0, 99.9);
  updateBarAndVerify(0, 100.0);
}

TEST_F(StatusbarUpdateTest, UpdateMultipleBarsInHandle) {
  int err = statusbar_log::destroy_statusbar_handle(this->handle);
  ASSERT_EQ(err, STATUSBARLOG_SUCCESS) << "Failed to destroy statusbar handle "
                                          "in UpdateMultipleBarsInHandle test";

  this->positions = {2, 1};
  this->bar_sizes = {50, 25};
  this->prefixes = {"second", "first"};
  this->postfixes = {"items", "things"};

  int err_code = statusbar_log::create_statusbar_handle(
      this->handle, this->positions, this->bar_sizes, this->prefixes,
      this->postfixes);

  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "Failed to create multiple statusbars in UpdateMultipleBarsInHandle "
         "test";

  updateBarAndVerify(0, 0.0);
  updateBarAndVerify(1, 0.0);
  updateBarAndVerify(0, 25.5);
  updateBarAndVerify(0, 50.0);
  updateBarAndVerify(1, 50.0);
  updateBarAndVerify(0, 75.0);
  updateBarAndVerify(1, 75.0);
  updateBarAndVerify(0, 100.0);
  updateBarAndVerify(1, 100.0);
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

  int err_code = statusbar_log::create_statusbar_handle(
      handle2, positions2, bar_sizes2, prefixes2, postfixes2);
  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "create_statusbar_handle should return STATUSBARLOG_SUCCESS";

  err_code = statusbar_log::destroy_statusbar_handle(handle2);

  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle2.valid)
      << "Handle should be marked as invalid after destruction";

  err_code = statusbar_log::update_statusbar(handle2, 0, 20);

  EXPECT_NE(err_code, STATUSBARLOG_SUCCESS)
      << "Should not be able to destroy already destroyed handle";
}

// ==================================================
// StatusbarValidations
// ==================================================

class StatusbarValidations : public ::testing::Test {
 protected:
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

    int err_code = statusbar_log::create_statusbar_handle(
        this->handle, this->positions, this->bar_sizes, this->prefixes,
        this->postfixes);

    ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
        << "Failed to create statusbar handle in fixture setup";
    ASSERT_TRUE(handle.valid)
        << "Handle should be valid after creation in fixture setup";
  }
};

TEST(StatusbarValidations, IsValidHandle) {
  statusbar_log::StatusbarHandle handle;
  statusbar_log::StatusbarHandle handle2;
  std::vector<unsigned int> positions = {1};
  std::vector<unsigned int> bar_sizes = {50};
  std::vector<std::string> prefixes = {"Processing"};
  std::vector<std::string> postfixes = {"items"};

  int err_code = statusbar_log::create_statusbar_handle(
      handle, positions, bar_sizes, prefixes, postfixes);
  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "Failed to create statusbar handle";
  ASSERT_TRUE(handle.valid) << "Handle should be valid after creation";

  err_code = statusbar_log::create_statusbar_handle(
      handle2, positions, bar_sizes, prefixes, postfixes);
  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "Failed to create statusbar handle";
  ASSERT_TRUE(handle2.valid) << "Handle should be valid after creation";

  handle.valid = false;
  err_code = statusbar_log::destroy_statusbar_handle(handle);
  EXPECT_EQ(err_code, -1);

  handle.valid = true;
  int idx_backup = handle.idx;
  handle.idx = SIZE_MAX;
  err_code = statusbar_log::destroy_statusbar_handle(handle);
  EXPECT_EQ(err_code, -2);

  handle.idx = 99999;
  err_code = statusbar_log::destroy_statusbar_handle(handle);
  EXPECT_EQ(err_code, -2);

  handle.idx = idx_backup + 1;
  err_code = statusbar_log::destroy_statusbar_handle(handle);
  EXPECT_EQ(err_code, -3);

  handle.idx = idx_backup;
  int ID_backup = handle.id;
  handle.id = handle2.id;
  err_code = statusbar_log::destroy_statusbar_handle(handle);
  EXPECT_EQ(err_code, -3);

  handle.id = ID_backup + 1;
  err_code = statusbar_log::destroy_statusbar_handle(handle);
  EXPECT_EQ(err_code, -3);

  // BUG : Cant test err_code -4

  handle.id = ID_backup;
  err_code = statusbar_log::destroy_statusbar_handle(handle);
  EXPECT_EQ(err_code, STATUSBARLOG_SUCCESS);
  err_code = statusbar_log::destroy_statusbar_handle(handle2);
  EXPECT_EQ(err_code, STATUSBARLOG_SUCCESS);
}
