
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
```zsh
cmake .. -DCMAKE_BUILD_TYPE=Release
```

```zsh
cmake --build . -j$(nproc) --config Release
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
- Mutexes for `statusbar_registry`, `statusbar_free_handles`, `handle_ID_count`
- Thread safety (test race conditions)
- Invalid handles (in update_statusbar and destroy_statusbar)
- Test unsanitised strings (in log and prefixes and postfixes)
- Test bounds of log message, filename, prefix and postfix length
- Test bounds on statusbar_registry and statusbar_free_handles

### TEST PRIORITY LIST
**HIGH PRIORITY - Core Functionality**
1 Handle Management Tests
```cpp
// StatusBar_handle lifecycle
// TEST(HandleManagement, CreateSingleBarHandle)
// TEST(HandleManagement, CreateMultiBarHandle) 
// TEST(HandleManagement, CreateHandle_InvalidInputSizes)
// TEST(HandleManagement, CreateHandle_MaxActiveHandlesLimit)
// TEST(HandleManagement, DestroyValidHandle)
// TEST(HandleManagement, DestroyInvalidHandle)
// TEST(HandleManagement, DestroyAlreadyDestroyedHandle)
```

2. Status Bar Update Tests
```cpp
// Basic updates
TEST(StatusBarUpdate, UpdateValidPercentage)
TEST(StatusBarUpdate, UpdateMultipleBarsInHandle)
TEST(StatusBarUpdate, UpdateWithInvalidPercentage_Negative)
TEST(StatusBarUpdate, UpdateWithInvalidPercentage_Over100)
TEST(StatusBarUpdate, UpdateWithInvalidIndex)
TEST(StatusBarUpdate, UpdateWithInvalidHandle)
```

3. Validation Function Tests
```cpp
// Internal validation logic
TEST(Validation, IsValidHandle_ValidCase)
TEST(Validation, IsValidHandle_InvalidFlag)
TEST(Validation, IsValidHandle_IndexOutOfBounds)
TEST(Validation, IsValidHandle_IDMismatch)
TEST(Validation, IsValidHandleVerbose_AllErrorCases)
```

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
TEST(Logging, LogWithoutStatusBars)
TEST(Logging, LogWithActiveStatusBars)
TEST(Logging, LogDifferentLevels)
TEST(Logging, LogWithFormatting)
TEST(Logging, LogWithLongMessages_Truncation)
```

7. Concurrent Access Tests

```cpp
TEST(Concurrency, MultipleHandlesSimultaneousCreation)
TEST(Concurrency, UpdatesFromDifferentThreads_NoCrash)
TEST(Concurrency, LogWhileStatusBarActive)
```

8. Edge Case & Stress Tests
```cpp
TEST(EdgeCases, RapidSequentialUpdates)
TEST(EdgeCases, ManyStatusBarsSimultaneously)
TEST(EdgeCases, EmptyVectors_ErrorHandling)
TEST(EdgeCases, VeryLongPrefixPostfix_Truncation)
TEST(EdgeCases, BoundaryPercentages_0_and_100)
```

**SPECIFIC TESTABLE BEHAVIORS**
For _draw_statusbar_component:
```cpp
TEST(DrawStatusBar, FormatsCorrectly_0Percent)
TEST(DrawStatusBar, FormatsCorrectly_50Percent)
TEST(DrawStatusBar, FormatsCorrectly_100Percent)
TEST(DrawStatusBar, SpinnerCyclesCorrectly)
TEST(DrawStatusBar, BarWidthRespected)
TEST(DrawStatusBar, TerminalWidthTruncation)
TEST(DrawStatusBar, ErrorCodes_AllScenarios)
```

For _get_terminal_width:
```cpp
TEST(TerminalWidth, DefaultsTo80OnFailure)
TEST(TerminalWidth, PlatformSpecificBehavior)
```

For create_statusbar_handle:
```cpp
TEST(CreateHandle, ReusesFreeHandles)
TEST(CreateHandle, SanitizesInputStrings)
TEST(CreateHandle, InitializesAllComponentsToZero)
TEST(CreateHandle, SetsHandleValidAndID)
```

For update_statusbar:
```cpp
TEST(UpdateStatusBar, SpinnerIndexIncrements)
TEST(UpdateStatusBar, ErrorReportedFlagSetsOnce)
TEST(UpdateStatusBar, NoErrorOnSubsequentUpdatesAfterError)
```
