# Controller Detection Experiment

This experiment demonstrates how to detect and log inputs from game controllers using SDL2.

## Features
- Detects all connected controllers
- Logs controller button presses
- Logs analog stick movements
- Logs trigger inputs
- Supports multiple controllers simultaneously

## Dependencies
- SDL2 (Simple DirectMedia Layer 2)
- SDL2 Game Controller support

## Building
```bash
# Install SDL2 in MSYS2
pacman -S mingw-w64-ucrt-x86_64-SDL2

# Build the experiment
cd build
cmake ..
cmake --build .
```

## Usage
Run the executable and it will:
1. List all connected controllers
2. Log all controller inputs in real-time
3. Press 'q' to quit the program

## Notes
- This is an experimental feature
- Currently supports XInput and DirectInput controllers
- Tested with Xbox and PlayStation controllers 