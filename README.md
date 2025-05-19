
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
