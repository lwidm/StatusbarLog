# -- StatusbarLog/tests/CMakeLists.txt

# =============================================================================
# Google Test
# =============================================================================
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
      DOWNLOAD_EXTRACT_TIMESTAMP
      FALSE)

# Only set gtest_force_shared_crt on Windows
if(WIN32)
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME}_test
               ${CMAKE_CURRENT_SOURCE_DIR}/src/StatusbarLog_test.cpp)

target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME}
                                                   GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)

# =============================================================================
# Main Testing Executable (Optional)
# =============================================================================

if(STATUSBARLOG_BUILD_TEST_MAIN)
  add_executable(${PROJECT_NAME}_main
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/StatusbarLog_main.cpp)
  target_link_libraries(${PROJECT_NAME}_main PRIVATE ${PROJECT_NAME})
endif()
