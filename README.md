# StatusbarLog

StatusbarLog is a C++ utility for simultaneous logging and multiple stacked statusbar displays in terminal applications.

## Features

- Multiple stacked statusbars with configurable text, sizes, and positions
- Logging with severity levels: ERROR, WARN, INFO, DEBUG
- Spinner animation for "busy" statusbars
- Cursor manipulation so log messages and statusbars do not overwrite each other
- Cross-platform design goals

## Building

### Quick build
```zsh
mkdir -p build && cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf
cmake --build . -j$(nproc) --config Release
```
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
## Requirements

- C++20 capable compiler (Clang, GCC, or MSVC)
- CMake ≥ 3.20

For Doxygen documentation:
- doxygen
- graphviz (for diagrams)

Install (Debian/Ubuntu):
```zsh
sudo apt install doxygen graphviz
```
Install (Arch)

## Documentation

Full documentation is available via Doxygen. To generate documentation locally:
```zsh
doxygen Doxyfile
```
HTML output is usually in docs/html.

For online documentation hosted via GitHub Pages, see the project's GitHub Pages site.

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
