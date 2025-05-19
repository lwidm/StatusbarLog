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
   ```cpp
    #define LOG_LEVEL LOG_LEVEL_INF
   ```
2. **Simple log message**:
   ```cpp
   LOG_INF(FILENAME, "Starting test...");
   ```
3. **StatusBar setup**:
   ```cpp
   StatusbarLog::StatusBar statusbar;
   StatusbarLog::g_statusbar_registry.push_back(&statusbar); // <- DO NOT FORGET
   ```
   Creates a status bar **and registers it globally**.
4. **StatusBar configuration**:
   ```cpp
   StatusbarLog::create_statusbar(
     statusbar, 
     {2, 1},      // Positions (2=top bar, 1=lower bar) 
     {20, 10},    // Bar widths
     {"first: ", "second: "}, 
     {" -- 50 steps", " -- 100 steps"}
   );
   ```
   Configures a two-bar stack with prefixes/postfixes.
5. **Updating progress**:
   ```cpp
   StatusbarLog::update_statusbar(statusbar, 0, percent);  // Update top bar
   StatusbarLog::update_statusbar(statusbar, 1, percent);  // Update lower bar
   ```
   Updates specific bars by index.
6. **Logging during updates**:
   ```cpp
   LOG_INF("main.cpp", "10 Ticks reached\n");
   ```
   Log messages appear above active status bars.

@section building Building
TODO
