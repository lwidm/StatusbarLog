cmake_minimum_required(VERSION 3.24)
project(statusbarlog)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_COMPILER "clang++")
# set(CMAKE_C_COMPILER "gcc") set(CMAKE_C_COMPILER "g++")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_FLAGS -O1 -Wall -Wextra -Wpedantic -Wno-unused-command-line-argument -Wformat)

# Add the source files.
set(SRC_FILES main.cpp StatusbarLog.cpp)
list(TRANSFORM SRC_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

# Build the executable
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMMON_FLAGS})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Add compile_commands.json from build directory
add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND
	${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_BINARY_DIR}/compile_commands.json"
	"${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json"
)
