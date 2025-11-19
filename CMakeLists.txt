# SPDX-License-Identifier: Apache-2.0 Copyright (c) 2025 Lukas Widmer

# -- statusbarlog/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(
  statusbarlog
  VERSION 1.1.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Configuration Options
# =============================================================================

option(STATUSBARLOG_INSTALL "Generate installation targets" OFF)
option(STATUSBARLOG_BUILD_TESTS "Build tests" OFF)
option(STATUSBARLOG_BUILD_TEST_MAIN "Build main executable for testing" OFF
)# (used in statusbarlog/tests/CMakeLists.txt)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Build type" FORCE)
endif()

# ----------- kLogLevel ---------

set(STATUSBARLOG_LOG_LEVEL
    "kLogLevelDbg"
    CACHE STRING "sets the kLogLevel global in the statusbar_log.h.in")

# validation
set(_valid_levels kLogLevelOff kLogLevelErr kLogLevelWrn kLogLevelInf
                  kLogLevelDbg)

list(FIND _valid_levels "${STATUSBARLOG_LOG_LEVEL}" _found_index)
if(_found_index EQUAL -1)
  message(
    FATAL_ERROR "Invalid STATUSBARLOG_LOG_LEVEL: ${STATUSBARLOG_LOG_LEVEL}. "
                "Valid values are: ${_valid_levels}")
endif()

# =============================================================================
# Library Target
# =============================================================================

# Generate header
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/statusbarlog/statusbarlog.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/statusbarlog/statusbarlog.h @ONLY)

# Add the library sources
set(SRC_FILES statusbarlog.cc sink.cc)
list(TRANSFORM SRC_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

# Create the library
add_library(${PROJECT_NAME} ${SRC_FILES})

# Include directories
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>)

message(STATUS "Detected C++ compiler: ${CMAKE_CXX_COMPILER_ID}")

# -----------------------------------------------------------------------------
# Compiler flags per build type
# -----------------------------------------------------------------------------
if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
endif()

if(MSVC)
  message(STATUS "Configuring for MSVC compiler")

  target_compile_options(${PROJECT_NAME} PRIVATE /std:c++20 /W4 /WX)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE /Zi /Od)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE /O2 /Ob2 /fp:fast)
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(${PROJECT_NAME} PRIVATE /O2 /Ob2 /Zi /DEBUG /fp:fast)
  endif()

else()
  message(STATUS "Configuring for GCC/Clang compiler")
  target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic
                                                 -Wformat)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -ffast-math)
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -g -ffast-math)
  endif()
endif()

# Print final configuration summary
message(STATUS "")
message(STATUS "StatusbarLog Configuration Summary:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  C++ Standard: 20")
message(STATUS "  Log Level: ${STATUSBARLOG_LOG_LEVEL}")
if(MSVC)
  message(STATUS "  MSVC Optimizations: /O2 /Ob2 /fp:fast (Release)")
else()
  message(STATUS "  GCC/Clang Optimizations: -O3 -ffast-math (Release)")
endif()
message(STATUS "")

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
# Copy compile_commands.json (Development convenience)
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
