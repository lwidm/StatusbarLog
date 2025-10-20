@mainpage statusbarLog Documentation

@section overview Overview

**StatusbarLog** is a C++ utility for simultaneous logging and multiple stacked statusbar displays in terminal applications.

Features:
- Multiple stacked statusbars with configurable text, sizes, and positions
- Logging with severity levels: `ERROR`, `WARN`, `INFO`, `DEBUG`
- Spinner animation for "busy" statusbars
- Cursor manipulation so log messages and statusbars do not overwrite each other
- Cross-platform design goals

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

### Quick build

```sh
mkdir -p build && cd build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf
cmake --build . -j$(nproc) --config Release
```

### Important CMake Options

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

@section requirements Requirements

- **C++20** capable compiler (Clang, GCC, or MSVC)
- **CMake ≥ 3.20**
- For Doxygen documentation:
  - `doxygen`
  - `graphviz` (for diagrams)

Install (Debian/Ubuntu):
```sh
sudo apt install doxygen graphviz
```
Install (Arch)
<!-- TODO: -->

If `dot` cannot be found, adjust the `DOT_PATH` in your `Doxyfile`:
```
DOT_PATH = /usr/bin/dot
```

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
