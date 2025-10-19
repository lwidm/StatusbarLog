
# StatusbarLog.cpp

## Overview
A c++ utility for simultaneously logging and statusbar displays in terminals.
Features:
- Multiple stacked statusbars with configurable text and positions
- Logging with severity levels (ERROR, WARN, INFO, DEBUG)
- Spinner animation for "busy" statusbar
- Cursor manipulation for seamless integration between logging messages and statusbar without overwriting each other.
- Doxygen documentation available

## Requirements
### Doxygen
In order to generate the doxygen documentation you need the following packages
- doxygen
- Graphviz

On **Debian** can run the following commands:
```zsh
sudo apt install doxygen graphviz
```

If you are using a different operating system you might need to modify the following line in the `DoxyFile`file:
```
DOT_PATH = /usr/bin/dot
```
The line above specifies where to find the `dot` executable of the graphviz program.

## Building

### Quick build
```zsh
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc) --config Release
```

### CMake options provided by this project (what each one means)

- `CMAKE_BUILD_TYPE` (STRING)  
  Standard CMake build type (Release, Debug, RelWithDebInfo, MinSizeRel).
  If not set and multi-config generators are not used, the project defaults to `Release`.

- `STATUSBARLOG_INSTALL` (BOOL)  
  Default: `OFF`.  
  If `ON`, generates installation targets for the library (install rules are present in the CMakeLists).
  Example:
```zsh
cmake -S . -B build -DSTATUSBARLOG_INSTALL=ON
cmake --install build --prefix /usr/local
```
- `STATUSBARLOG_BUILD_TESTS` (BOOL)  
  Default: `OFF`.  
  If `ON`, `enable_testing()` is invoked and the `tests/` subdirectory is processed to add tests.
  Enable:
```zsh
cmake -S . -B build -DSTATUSBARLOG_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

- `STATUSBARLOG_BUILD_TEST_MAIN` (BOOL)
  Default: `OFF`.
  Used by `statusbarlog/tests/CMakeLists.txt` to control whether the test `main` executable is built
  Example:
```zsh
cmake -S . -B build -DSTATUSBARLOG_BUILD_TESTS=ON -DSTATUSBARLOG_BUILD_TEST_MAIN=ON
```

- `STATUSBARLOG_LOG_LEVEL` (STRING) 
  Default: `kLogLevelDbg`.
  Controls the compile-time default `kLogLevel` value generated into the header by `configure_file()`.
  Allowed values (must match the enum): `kLogLevelOff`, `kLogLevelErr`, `kLogLevelWrn`, `kLogLevelInf`, `kLogLevelDbg`. 
  Set via CLI:
```zsh
cmake -S . -B build -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf
```
  Or when consuming via `add_subdirectory()` set it before adding the subdirectory:
```cmake
set(STATUSBARLOG_LOG_LEVEL kLogLevelWrn CACHE STRING "statusbarlog default")
add_subdirectory(path/to/statusbarlog)
```
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
