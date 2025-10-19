@mainpage StatusbarLog Module Documentation

@section overview Overview
A c++ utility for simultaneously logging and statusbar displays in terminals.
Features:
- Multiple stacked statusbars with configurable text and positions
- Logging with severity levels (ERROR, WARN, INFO, DEBUG)
- Spinner animation for "busy" statusbar
- Cursor manipulation for seamless integration between logging messages and statusbar without overwriting each other.

@section usage_example Usage Example
The following code from `main.cpp` shows a simple example use-case:

@include main.cpp

@subsection explanation Code example explanation:
1. **Set global logging threshold**:
<!-- TODO : -->
```cpp
```
1b). **At the top of every cpp file define the filename** TODO: should be replace with constexpr
```cpp
#define FILENAME "main.cpp"
```
2. **Simple log message**:
```cpp
statusbar_log::LogInf(FILENAME, "Starting test...");
```
3. **Statusbar setup**:
  ```cpp
statusbar_log::StatusbarHandle handle;
std::vector<unsigned int> positions = {2, 1};
std::vector<unsigned int> bar_sizes = {20, 10};
std::vector<std::string> prefixes = {"first", "second"};
std::vector<std::string> postfixes = {"20 long", "10 long"};

int err_code = statusbar_log::CreateStatusbarHandle(
   handle, positions, bar_sizes, prefixes, postfixes);
```
   Configures a two-bar stack with prefixes/postfixes.
4. **Updating progress**:
```cpp
statusbar_log::UpdateStatusbar(statusbar, 0, percent);  // Update top bar
statusbar_log::UpdateStatusbar(statusbar, 1, percent);  // Update lower bar
```
   Updates specific bars by index.
5. **Logging during updates**:
```cpp
statusbar_log::LogInf("main.cpp", "10 Ticks reached\n");
```
   Log messages appear above active status bars.

6. **Cleanup - DO NOT FORGET**
   After a statusbar is no longer needed don't forget to clean it up
```cpp
int err_code = statusbar_log::DestroyStatusbarHandle(handle);
```

@section building Building
TODO
