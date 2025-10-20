@mainpage statusbarLog Documentation


@section overview Overview

**StatusbarLog** is a C++ utility for simultaneous logging and multiple stacked statusbar displays in terminal applications.

Features:
- Multiple stacked statusbars with configurable text, sizes, and positions
- Logging with severity levels: `ERROR`, `WARN`, `INFO`, `DEBUG`
- Spinner animation for "busy" statusbars
- Cursor manipulation so log messages and statusbars do not overwrite each other
- Cross-platform design goals

@tableofcontents

@section usage_example Usage Example

Example code snippet (from `docs/example/main.cpp`):

@include docs/example/main.cpp

@subsection brief Brief explenation

1. **Set compile-time log level (optional)**  
   The log level is set by CMake when generating the header. This ensure only log messages with higher or equal log priority (ERROR, WARNING, INFO, DEBUG) to the log level will be printed.
   Override via:
   ```sh
   cmake -DSTATUSBARLOG_LOG_LEVEL=kLogLevelWrn ...
   ```
   (all options: `kLogLevelDbg`, `kLogLevelInf`, `kLogLevelWrn`, `kLogLevelErr`, `kLogLevelOff`)

2. **Define filename for every cpp file in which you want to log** 
   ```cpp
   const std::string kFilename = "StatusbarLog_main.cpp";
   ```

3. **Now a simple log message can be done like:**
   ```cpp
   statusbarlog::LogDbg(kFilename, "Funny debug message");
   statusbarlog::LogInf(kFilename, "Starting test...");
   statusbarlog::LogWrn(kFilename, "Couldn't obtain viscosity. Using 1.6e-5 m^2/s");
   statusbarlog::LogErr(kFilename, "Failed to compute rhs");
   ```

4. **Create a stacked statusbar** (here: two statusbars ontop of each other)
   ```cpp
   statusbarlog::StatusbarHandle handle;
   std::vector<unsigned int> positions = {2, 1};
   std::vector<unsigned int> bar_sizes = {20, 10};
   std::vector<std::string> prefixes = {"first", "second"};
   std::vector<std::string> postfixes = {"20 long", "10 long"};

   int err_code = statusbarlog::CreateStatusbarHandle(
       handle, positions, bar_sizes, prefixes, postfixes);
   ```

5. **Updating a statusbar**
   ```cpp
   statusbarlog::UpdateStatusbar(handle, 0, percent);  // top bar
   statusbarlog::UpdateStatusbar(handle, 1, percent);  // lower bar
   ```
   Note: For printing the statusbar the first time just use the percentage 0.

6. **Log while updating**
   ```cpp
   statusbarlog::LogInf(kFilename, "10 ticks reached");
   ```
   The log messages now nicely display above the statusbar.

7. **Cleanup**
   ```cpp
   int err_code = statusbarlog::DestroyStatusbarHandle(handle);
   ```

@section building Building

@subsection prerequisites prerequisites

@subsubsection prereq_all_platforms All Platforms:
- C++20 capable compiler (Clang, GCC, or MSVC)
- CMake >= 3.20@subsubsection prereq_all_platforms All Platforms:

For Doxygen documentation:
- doxygen
- graphviz (for diagrams)

@subsubsection prereq_deb Linux (Debian/Ubuntu):
```zsh
sudo apt install build-essential cmake
```
For Doxygen documentation:
```zsh
sudo apt install doxygen graphviz
```
@subsubsection prereq_arch Linux (Arch):
```zsh
sudo pacman -S base-devel cmake
```
For Doxygen documentation:
```zsh
sudo pacman -S base-devel doxygen graphviz
```

@subsubsection prereq_windows Windows:
- Visual Studio 2019 or later with individual components:
   - MSBuild
   - Windows 11 SDK
   - MSBuild support for LLVM (clang-cl) toolset
   - C++ Clang compiler for Windows
   - MSVC v143 - VS 2022 C++ ARM build tools (Latest)
   - MSVC v143 - VS 2022 C++ ARM Spectre-mitigated libs (Latest)
   - MSVC v143 - VS 2022 C++ ARM64/ARM64EC build tools (Latest)
   - MSVC v143 - VS 2022 C++ ARM64/ARM64EC Spectre-mitigated libs (Latest)
   - MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
   - MSVC v143 - VS 2022 C++ x64/x86 Spectre-mitigated libs (Latest)
   - C++ Cmake tools for Windows
   - C++ Cmake tools for Linux
   (These are just the components i have installed, not all might be required and others might work)
- Cmake (can be installed manually or using visual studio)
- Ninja
```PowerShell
winget install Ninja-build.Ninja
```
- For Dxygen documentation:
   - Doxygen
   - graphviz
```PowerShell
winget install doxygen
winget install graphviz
```
- **Don't forget to add the graphvize binaries to PATH!** (usually located at `C:/Program Files/Graphviz/bin`)

@subsection build_linux Building on Linux/macOS

```sh
mkdir -p build && cd build
cmake -S .. -B . build -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf
cmake --build . -j$(nproc) --config Release
```

@subsection build_windows Building on Windows

@subsubsection windows_method1 Method 1: Using Visual Studio Developer Command Prompt
open "Developer Command Promt for VS 2022" or similar
```cmd
mkdir build
cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf
cmake --build . --config Release --parallel
```
@subsubsection windows_method2 Method 2: Using Ninja (Recommended)
```cmd
mkdir build
cd build
cmake -S .. -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf
cmake --build . --parallel
```
@subsubsection windows_method3 Method 3: Using Visual Studio IDE
```cmd
mkdir build
cd build
cmake -S .. -B . -G "Visual Studio 17 2022" -A x64
```
Then open the generated `.sln` in Visual Studio

@subsection cmake_options Important CMake Options

| Option | Type | Default | Description |
|--------|------|----------|--------------|
| `CMAKE_BUILD_TYPE` | STRING | `Release` | Standard CMake build type |
| `STATUSBARLOG_INSTALL` | BOOL | `OFF` | Generate installation targets |
| `STATUSBARLOG_BUILD_TESTS` | BOOL | `OFF` | Build test suite |
| `STATUSBARLOG_BUILD_TEST_MAIN` | BOOL | `OFF` | Build test main executable |
| `STATUSBARLOG_LOG_LEVEL` | STRING | `kLogLevelDbg` | Compile-time log level (`kLogLevelOff`, `kLogLevelErr`, `kLogLevelWrn`, `kLogLevelInf`, `kLogLevelDbg`) |

Example usage:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelDbg
```

Or when consuming via `add_subdirectory()`:
```cmake
set(STATUSBARLOG_LOG_LEVEL kLogLevelWrn CACHE STRING "statusbarlog default")
add_subdirectory(path/to/statusbarlog)
```

@subsection performance_notes_windows Performance Notes for Windows
- Use Ninja generator for fastest build times
- MSVC compiler with */O2* and *LTO* provides best runtime performance
- Consider *Profile-Guided Optimization (PGO)* for maximum performance in release builds

@section generating_docs Generating Documentation

1. Ensure Doxygen and Graphviz are installed  
2. Adjust the Doxyfile to point to this mainpage:  
   ```
   INPUT = src include docs
   FILE_PATTERNS = *.h *.hpp *.cpp *.md
   MAINPAGE = docs/mainpage.md
   EXCLUDE_PATTERNS = README.md
   ```
3. Run:
   ```sh
   doxygen Doxyfile
   ```

HTML output is usually in `docs/html`.

@section roadmap TODO / Roadmap

- ✅ CMake integration with configurable `STATUSBARLOG_LOG_LEVEL`
- 🔧 Make usable as git submodule / cmake module
- 🔧 Stream selection for logs and statusbars
- 🔧 Optional flush control for performance
- ⚙️ Add cross-platform support (Windows)
- 🧪 Expand unit test coverage:
  - Destroying handles / invalid handles
  - Out-of-bounds indices
  - Mutex and thread safety
  - Truncation and string sanitization
  - Race conditions
  - Edge cases and boundary tests
- 📘 Follow Google C++ Style Guide:
  - Replace macros with `constexpr` or inline functions
  - Consistent naming: PascalCase for functions, UpperCamel for types
  - Avoid global state

@section testing_unit Unit Test Plan (Summary)

**String & Utility Functions**
```cpp
TEST(StringSanitization, SanitizeString_NoChangesNeeded)
TEST(StringSanitization, SanitizeString_ControlCharacters)
TEST(StringSanitization, SanitizeString_UnicodeReplacement)
TEST(StringSanitization, SanitizeStringWithNewline_PreservesNewlines)
TEST(StringSanitization, SanitizeString_TruncatesLongStrings)
```

**Terminal Utility Tests**
```cpp
TEST(TerminalUtils, GetTerminalWidth_Success)
TEST(TerminalUtils, GetTerminalWidth_FallbackToDefault)
TEST(TerminalUtils, ClearCurrentLine_NoCrash)
TEST(TerminalUtils, CursorPosition_SaveRestore)
TEST(TerminalUtils, FlushOutput_Behavior)
```

**Integration Tests**
```cpp
TEST(Logging, LogWithActiveStatusbars)
TEST(Logging, LogDifferentLevels)
TEST(Logging, LogWithFormatting)
TEST(Logging, LogWithLongMessages_Truncation)
```

**Concurrency**
```cpp
TEST(Concurrency, MultipleHandlesSimultaneousCreation)
TEST(Concurrency, UpdatesFromDifferentThreads_NoCrash)
TEST(Concurrency, LogWhileStatusbarActive)
```

**Edge Cases**
```cpp
TEST(EdgeCases, RapidSequentialUpdates)
TEST(EdgeCases, ManyStatusbarsSimultaneously)
TEST(EdgeCases, EmptyVectors_ErrorHandling)
TEST(EdgeCases, VeryLongPrefixPostfix_Truncation)
TEST(EdgeCases, BoundaryPercentages_0_and_100)
```
```
