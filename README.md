# StatusbarLog

StatusbarLog is a C++ utility for simultaneous logging and multiple stacked statusbar displays in terminal applications.

## Features

- Multiple stacked statusbars with configurable text, sizes, and positions
- Logging with severity levels: ERROR, WARN, INFO, DEBUG
- Spinner animation for "busy" statusbars
- Cursor manipulation so log messages and statusbars do not overwrite each other
- Cross-platform design goals

## Building

### prerequisites

#### All Platforms:
- C++20 capable compiler (Clang, GCC, or MSVC)
- CMake >= 3.20@subsubsection prereq_all_platforms All Platforms:

For Doxygen documentation:
- doxygen
- graphviz (for diagrams)

#### Linux (Debian/Ubuntu):
```zsh
sudo apt install build-essential cmake
```
For Doxygen documentation:
```zsh
sudo apt install doxygen graphviz
```

#### Linux (Arch):
```zsh
sudo pacman -S base-devel cmake
```
For Doxygen documentation:
```zsh
sudo pacman -S base-devel doxygen graphviz
```

#### Windows:
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

### Building on Linux/macOS

```sh
mkdir -p build && cd build
cmake -S .. -B . build -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf
cmake --build . -j$(nproc) --config Release
```

### Building on Windows

#### Method 1: Using Visual Studio Developer Command Prompt
open "Developer Command Promt for VS 2022" or similar
```cmd
mkdir build
cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf
cmake --build . --config Release --parallel
```

#### Method 2: Using Ninja (Recommended)
```cmd
mkdir build
cd build
cmake -S .. -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf
cmake --build . --parallel
```
#### Method 3: Using Visual Studio IDE
```cmd
mkdir build
cd build
cmake -S .. -B . -G "Visual Studio 17 2022" -A x64
```
Then open the generated `.sln` in Visual Studio

### Important CMake Options

| Option | Type | Default | Description |
|--------|------|----------|--------------|
| CMAKE_BUILD_TYPE | STRING | Release | Standard CMake build type |
| STATUSBARLOG_INSTALL | BOOL | OFF | Generate installation targets |
| STATUSBARLOG_BUILD_TESTS | BOOL | OFF | Build test suite |
| STATUSBARLOG_BUILD_TEST_MAIN | BOOL | OFF | Build test main executable |
| STATUSBARLOG_LOG_LEVEL | STRING | kLogLevelDbg | Compile-time log level (kLogLevelOff, kLogLevelErr, kLogLevelWrn, kLogLevelInf, kLogLevelDbg) |

Example usage:
```zsh
cmake -S .. -B build -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelDbg
```
Or when consuming via add_subdirectory():
```cmake
set(STATUSBARLOG_LOG_LEVEL kLogLevelWrn CACHE STRING "statusbarlog default")
add_subdirectory(path/to/statusbarlog)
```

### Performance Notes for Windows
- Use Ninja generator for fastest build times
- MSVC compiler with */O2* and *LTO* provides best runtime performance
- Consider *Profile-Guided Optimization (PGO)* for maximum performance in release builds

## Documentation

Full documentation is available via Doxygen. To generate documentation locally:
```zsh
doxygen Doxyfile
```
HTML output is usually in docs/html.

For online documentation hosted via GitHub Pages, see the project's [GitHub Pages site](https://lwidm.github.io/statusbarlog).

## Devopment
I used the compilation database located at `compile_commands.json.in` together with the `clangd` lsp for development and this works very well. I recommend using a build system that supports generating this compilation database (like _make_ or _Ninja_). If a build system that supports it is used cmake will generate the compilation database for you and coppy it to the root directory. If one doesn't want to do this one can coppy the `compile_commands.json.windows` or `compile_commands.json.linux` to `compile_commands.json` and place it in the root directory. This should work just fine as long as no new files or defines are created.

## TODO
- Make usable as git submodule and cmake module
- Let log messages and statusbars take up arbitrary streams
- Optionally don't force flushing after every status message or every statusbarupdate
- Cross platform compatibility
    - Windows
    - Linux
- Read through google's style guide
    - Reduce Macros
    - functions are Pascalcase ?
    - Classes are ___? 
    - etc...

## TODO unittest
- destroying of statusbar (all error codes)
- out of bounds `spin_idx`, log message length, status bar text, `statusbar_registry`
- Mutexes for `statusbar_registry`, `statusbar_free_handles`, `handle.id_count`
- Thread safety (test race conditions)
- Invalid handles (in UpdateStatusbar and destroy_statusbar)
- Test unsanitised strings (in log and prefixes and postfixes)
- Test bounds of log message, filename, prefix and postfix length
- Test bounds on statusbar_registry and statusbar_free_handles

### TEST PRIORITY LIST

**MEDIUM PRIORITY - String & Utility Functions**
4. String Sanitization Tests 
```cpp
TEST(StringSanitization, SanitizeString_NoChangesNeeded)
TEST(StringSanitization, SanitizeString_ControlCharacters)
TEST(StringSanitization, SanitizeString_UnicodeReplacement)
TEST(StringSanitization, SanitizeStringWithNewline_PreservesNewlines)
TEST(StringSanitization, SanitizeString_TruncatesLongStrings)
TEST(StringSanitization, SanitizePrefixPostfixLengthLimits)
```

5. Terminal Utility Tests
```cpp
TEST(TerminalUtils, GetTerminalWidth_Success)
TEST(TerminalUtils, GetTerminalWidth_FallbackToDefault)
TEST(TerminalUtils, ClearCurrentLine_NoCrash)
TEST(TerminalUtils, CursorPosition_SaveRestore)
TEST(TerminalUtils, FlushOutput_Behavior)
```

**LOW PRIORITY - Complex Scenarios**
6. Logging Integration Tests
```cpp
TEST(Logging, LogWithoutStatusbars)
TEST(Logging, LogWithActiveStatusbars)
TEST(Logging, LogDifferentLevels)
TEST(Logging, LogWithFormatting)
TEST(Logging, LogWithLongMessages_Truncation)
```

7. Concurrent Access Tests

```cpp
TEST(Concurrency, MultipleHandlesSimultaneousCreation)
TEST(Concurrency, UpdatesFromDifferentThreads_NoCrash)
TEST(Concurrency, LogWhileStatusbarActive)
```

8. Edge Case & Stress Tests
```cpp
TEST(EdgeCases, RapidSequentialUpdates)
TEST(EdgeCases, ManyStatusbarsSimultaneously)
TEST(EdgeCases, EmptyVectors_ErrorHandling)
TEST(EdgeCases, VeryLongPrefixPostfix_Truncation)
TEST(EdgeCases, BoundaryPercentages_0_and_100)
```

**SPECIFIC TESTABLE BEHAVIORS**
For _DrawStatusbarComponent:
```cpp
TEST(DrawStatusbar, FormatsCorrectly_0Percent)
TEST(DrawStatusbar, FormatsCorrectly_50Percent)
TEST(DrawStatusbar, FormatsCorrectly_100Percent)
TEST(DrawStatusbar, SpinnerCyclesCorrectly)
TEST(DrawStatusbar, BarWidthRespected)
TEST(DrawStatusbar, TerminalWidthTruncation)
TEST(DrawStatusbar, ErrorCodes_AllScenarios)
```

For _GetTerminalWidth:
```cpp
TEST(TerminalWidth, DefaultsTo80OnFailure)
TEST(TerminalWidth, PlatformSpecificBehavior)
```

For CreateStatusbarHandle:
```cpp
TEST(CreateHandle, ReusesFreeHandles)
TEST(CreateHandle, SanitizesInputStrings)
TEST(CreateHandle, InitializesAllComponentsToZero)
TEST(CreateHandle, SetsHandleValidAndID)
```

For UpdateStatusbar:
```cpp
TEST(UpdateStatusbar, SpinnerIndexIncrements)
TEST(UpdateStatusbar, ErrorReportedFlagSetsOnce)
TEST(UpdateStatusbar, NoErrorOnSubsequentUpdatesAfterError)
```
