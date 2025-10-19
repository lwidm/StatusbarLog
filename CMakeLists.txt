# -- statusbarlog/CMakeLists.txt

cmake_minimum_required(VERSION 3.15)
project(
  statusbarlog
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Configuration Options
# =============================================================================

option(STATUSBARLOG_INSTALL "Generate installation targets" OFF)
option(STATUSBARLOG_BUILD_TESTS "Build tests" OFF)
option(STATUSBARLOG_BUILD_TEST_MAIN "Build main executable for testing" OFF) # (used in statusbarlog/tests/CMakeLists.txt)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Build type" FORCE)
endif()

# =============================================================================
# Library Target
# =============================================================================

# Add the library sources
set(SRC_FILES statusbarlog.cpp)
list(TRANSFORM SRC_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

# Create the library
add_library(${PROJECT_NAME} ${SRC_FILES})

# Include directories
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include>)

# Set c++ standard
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_options(
  ${PROJECT_NAME} PRIVATE -std=c++20
                               $<$<CXX_COMPILER_ID:GNU>:-fno-extensions>)

# Compiler Flags (PRIVATE to avoid affecting parent projects)
target_compile_options(
  ${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU,CLANG,AppleClang>:-Wall
                          -Wextra -Wpedantic -Wformat>)

# =============================================================================
# Testing (Optional)
# =============================================================================

if(STATUSBARLOG_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# =============================================================================
# Installation (Optional)
# =============================================================================

if(STATUSBARLOG_INSTALL)
  install(
    TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/StatusbarLog.h
          DESTINATION include)
endif()

# =============================================================================
# Export for Parent Project
# =============================================================================

# Exports targets for use by parent project
export(TARGETS ${PROJECT_NAME}
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

# Generate package version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

# =============================================================================
# Copy compile_commands.json (Development conveniece)
# =============================================================================

# Only do this for the main project, not when used as submodule
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_BINARY_DIR}/compile_commands.json"
      "${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to source directory")
endif()
