#include "StatusbarLog/StatusbarLog.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

class HandleManagementTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(HandleManagementTest, CreateSingleBarHandle) {
  StatusbarLog::StatusBar_handle handle;
  std::vector<unsigned int> positions = {1};
  std::vector<unsigned int> bar_sizes = {50};
  std::vector<std::string> prefixes = {"Processing"};
  std::vector<std::string> postfixes = {"items"};

  int err_code = StatusbarLog::create_statusbar_handle(
      handle, positions, bar_sizes, prefixes, postfixes);

  ASSERT_EQ(err_code, STATUSBARLOG_SUCCESS)
      << "create_statusbar_handle should return STATUSBARLOG_SUCCESS";
  EXPECT_TRUE(handle.valid)
      << "Handle should be marked as valid after creation";
  EXPECT_NE(handle.ID, 0) << "Handle should have a non-zero ID assigned";
  EXPECT_LT(handle.ID, SIZE_MAX) << "Handle index should be a valid value";
  EXPECT_LT(handle.idx, MAX_ACTIVE_HANDLES)
      << "Handle index should be less than MAX_ACTIVE_HANDLES";
  EXPECT_EQ(StatusbarLog::update_statusbar(handle, 0, 50.0),
            STATUSBARLOG_SUCCESS)
      << "Should be able to update the status bar with valid handle";
  EXPECT_EQ(StatusbarLog::destroy_statusbar_handle(handle),
            STATUSBARLOG_SUCCESS)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle.valid)
      << "Handle should be marked as invalid after destruction";
}

TEST_F(HandleManagementTest, CreateMultiBarHandle) {
  StatusbarLog::StatusBar_handle handle;
  std::vector<unsigned int> positions = {2, 1};
  std::vector<unsigned int> bar_sizes = {20, 10};
  std::vector<std::string> prefixes = {"first", "second"};
  std::vector<std::string> postfixes = {"20 long", "10 long"};

  int err_code = StatusbarLog::create_statusbar_handle(
      handle, positions, bar_sizes, prefixes, postfixes);

  EXPECT_NE(handle.ID, 0) << "Handle should have a non-zero ID assigned";
  EXPECT_LT(handle.ID, SIZE_MAX) << "Handle index should be a valid value";
  EXPECT_LT(handle.idx, MAX_ACTIVE_HANDLES)
      << "Handle index should be less than MAX_ACTIVE_HANDLES";
  EXPECT_EQ(StatusbarLog::update_statusbar(handle, 0, 50.0),
            STATUSBARLOG_SUCCESS)
      << "Should be able to update the status bar with valid handle";
  EXPECT_EQ(StatusbarLog::destroy_statusbar_handle(handle),
            STATUSBARLOG_SUCCESS)
      << "Should be able to destroy the handle cleanly";
  EXPECT_FALSE(handle.valid)
      << "Handle should be marked as invalid after destruction";
}
