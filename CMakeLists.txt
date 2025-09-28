cmake_minimum_required(VERSION 3.24)
project(statusbarlog)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_C_COMPILER)
	find_program(CLANG_C clang)
	message(WARNING " -DCMAKE_C_COMPILER not specified, falling back to clang")
	if(CLANG_C)
		message(INFO " Using clang found at ${CLANG_C}")
		set(CMAKE_C_COMPILER "${CLANG_C}" CACHE STRING "C compiler" FORCE)
	else()
		message(WARNING " No clang compiler found, falling back to gcc")
		find_program(GCC_C gcc)
		if(GCC_C)
			message(INFO " Using gcc found at ${GCC_C}")
			set(CMAKE_C_COMPILER "${GCC_C}" CACHE STRING "C compiler" FORCE)
		else()
			message(FATAL_ERROR " No c compiler found")
		endif()
	endif()
else()
	message(INFO " Using c compiler found at ${CMAKE_C_COMPILER}")
endif()
if(NOT CMAKE_CXX_COMPILER)
	find_program(CLANG_CXX clang++)
	message(WARNING " -DCMAKE_CXX_COMPILER not specified, falling back to clang++")
	if(CLANG_CXX)
		message(INFO " Using clang++ found at ${CLANG_CXX}")
		set(CMAKE_CXX_COMPILER "${CLANG_CXX}" CACHE STRING "CXX compiler" FORCE)
	else()
		message(WARNING " No clang++ compiler found, falling back to g++")
		find_program(GPP_CXX g++)
		if(GPP_CXX)
			message(INFO " Using g++ found at ${GPP_CXX}")
			set(CMAKE_CXX_COMPILER "${GPP_CXX}" CACHE STRING "CPP compiler" FORCE)
		else()
			message(FATAL_ERROR " No c++ compiler found")
		endif()
	endif()
else()
	message(INFO " Using c++ compiler found at ${CMAKE_CXX_COMPILER}")
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_FLAGS -O1 -Wall -Wextra -Wpedantic -Wno-unused-command-line-argument
                 -Wformat)

# Add Cmake
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

# Add the source files.
set(SRC_FILES main.cpp StatusbarLog.cpp)
list(TRANSFORM SRC_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

# Build the executable
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMMON_FLAGS})
target_include_directories(${PROJECT_NAME}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Add compile_commands.json from build directory
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json")
