<!-- spdx-license-identifier: apache-2.0 -->
<!-- Copyright (c) 2025 Lukas Widmer -->
# StatusbarLog v1.0.1

A C++20 utility for simultaneous logging and multiple stacked statusbar displays in terminal applications.

##  Changelog
- First stable release

## What's Included

### 1. Full Source Package (`statusbarlog-full-v1.0.1`)
- Complete source code with tests and documentation
- Ready-to-use CMake module - just extract and add to your project
- Includes Doxygen configuration for generating documentation
- (Personally I recommend cloning the repo instead)

### 2. Minimal Package (`statusbarlog-minimal-v1.0.1`) 
- Source files, header files and CMake configuration
- Ready-to-use CMake module - just extract and add to your project

### 3. Individual Components
#### Header Files:
- `statusbarlog.h.in` - Template header with CMake placeholder `@STATUSBARLOG_LOG_LEVEL@`
- `statusbarlog.h` - Configured header with log level set to `kLogLevelInf`
#### Precompiled Libraries (built with `kLogLevelInf`):
- Linux x86-64: `statusbarlog-v1.0.1-linux-x86_64.a` (Clang, Release build)
- Windows x86-64: `TODO` (MSVC, Release build)

## License
- Apache 2.0 - See LICENSE file for details.
