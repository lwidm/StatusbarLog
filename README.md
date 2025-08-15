
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
- No bound check in `spin_idx`
- No range enforcement in `percent`
- Thread race conditions: 
- no wrap protection in `handle_ID_count`
- Make usable as git submodule and cmake module
- Use a constant representing sucess (STATUSBARLOG_SUCCESS := 0)
- Let log messages and statusbars take up arbitrary streams
- Optionally don't force flushing after every status message or every statusbarupdate

## TODO unittest
- destroying of statusbar (all error codes)
- out of bounds `spin_idx`, log message length, status bar text, `statusbar_registry`
- Mutexes for `statusbar_registry`, `statusbar_free_handles`, `handle_ID_count`
- Thread safety (test race conditions)
- Invalid handles (in update_statusbar and destroy_statusbar)
- Test unsanitised strings (in log and prefixes and postfixes)
- Test bounds of log message, filename, prefix and postfix length
- Test bounds on statusbar_registry and statusbar_free_handles
