@page cmake_module_page StatusbarLog as CMake (Sub-)Module

You can include **StatusbarLog** in your own C++ project in a few ways, I use it as a **git submodule** or by directly adding it to your project tree. Once included, you can consume it via CMake.

@section cmake_module_git_submodule 1. Include StatusbarLog as a submodule:
@code{bash}
git submodule add git@github.com:lwidm/statusbarlog.git statusbarlog
git submodule update --init --recursive
@endcode

@section cmake_module_consume 2. Add it to your CMake project
in your root `CMakeLists.txt`:
@code{cmake}
cmake_minimum_required(VERSION 3.20)
project(MyProject CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: set default StatusbarLog log level
set(STATUSBARLOG_LOG_LEVEL kLogLevelInf CACHE STRING "Default StatusbarLog log level")

# Include StatusbarLog
add_subdirectory(statusbarlog)

# Your executable
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE statusbarlog)
@endcode

@section cmake_module_build 3. Build your project:
@code{bash}
mkdir -p build
cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
@endcode

@section cmake_module_use 4. Use StatusbarLog in your code:
@code{cpp}
#include "statusbarlog/statusbarlog.h"

int main() {
    statusbarlog::LogInf("main.cpp", "Starting application...");
}
@endcode

@section cmake_module_notes Notes
- This approach allows you to keep StatusbarLog version-controlled alongside your project via the submodule,
- Or to copy the source folder directly into your project.
- When cloning your project elsewhere, run git submodule update --init --recursive if you use the submodule method.
