
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
- `destroy_statusbar_handle`
- Terminal width handling (`get_terminal_width`) Truncate statusbar text when wider than terminal
- Missing Escape code management
   ```cpp
   void save_cursor_position();
   void restore_cursor_position();
   void clear_to_end_of_line();
   ```
- Log message Truncation: No bound checking for log messages
- No message length limiting
- Multi line log handling: Current implementation fails to calculate total lines used by multi-line logging, properly redraw statusbar after multi-line log.
- Handle validation (no rebust validation):
   ```cpp
   bool is_valid_handle(const StatusBar_handle& h) {
        return h.idx < statusbar_registry.size() && 
        statusbar_registry[h.idx].ID == h.ID;
   }
   ```
- Thread safety: No mutexes for `statusbar_registry`, `statusbar_free_handles`, `handle_ID_count`
- Sanitise string before logging to not include control characters (maybe excluding \n)
- Unbounded vector growth on: `statusbar_registry`, log message length, Status bar text
- No bound check in `spin_idx`
- No range enforcement in `percent`
- Thread race conditions: 
   ```cpp
   handle_ID_count++;  // Non-atomic increment
   statusbar_registry.push_back(...);  // Non-locked
   ```
- sanitise prefixes and postfixes in statusbar
- no wrap protection in `handle_ID_count`
- Make usable as git submodule and cmake module

