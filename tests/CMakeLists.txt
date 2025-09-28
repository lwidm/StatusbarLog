# -- StatusbarLog/tests/CmakeLists.txt


# ========== Google Test ==========
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# Only set gtest_force_shared_crt on Windows
if(WIN32)
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

add_executable(statusbarLog_test
               ${CMAKE_CURRENT_SOURCE_DIR}/src/StatusbarLog_test.cpp)

target_link_libraries(statusbarLog_test PRIVATE ${PROJECT_NAME}
                                                GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(statusbarLog_test)


# ========== Main file for testing ==========
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_FLAGS -O1 -Wall -Wextra -Wpedantic -Wno-unused-command-line-argument
                 -Wformat)

# Add the source files.
set(SRC_FILES StatusbarLog_main.cpp)
list(TRANSFORM SRC_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

# Build the executable
add_executable(${PROJECT_NAME}_main ${SRC_FILES})
target_link_libraries(${PROJECT_NAME}_main PRIVATE ${PROJECT_NAME})
target_compile_options(${PROJECT_NAME}_main PRIVATE ${COMMON_FLAGS})
target_include_directories(${PROJECT_NAME}_main
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")
