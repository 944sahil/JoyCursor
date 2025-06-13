# Controller Mouse Mapping

This project provides functionality to map controller inputs to mouse movements and clicks. It allows you to use your game controller as a mouse input device.

## Features

- Right analog stick for fast mouse movement
- Left analog stick for precise mouse movement
- A button for left mouse click
- B button for right mouse click
- Configurable sensitivity and deadzone

## Building the Project

### Prerequisites

- CMake (version 3.10 or higher)
- SDL2 development libraries
- C++ compiler with C++17 support

### Build Steps

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Generate build files:
```bash
cmake ..
```

3. Build the project:
```bash
cmake --build .
```

The executables will be created in the `build/bin` directory.

## Usage

1. Run the mouse mapper:
```bash
./bin/mouse_mapper
```

2. Use the controller:
- Right stick: Fast mouse movement
- Left stick: Precise mouse movement
- A button: Left click
- B button: Right click

3. To exit the program, press the X button on the window or use CTRL+C.

## Notes

- The program requires a compatible game controller to be connected
- Mouse movement sensitivity can be adjusted by modifying the constants in the source code
- The program uses Windows API for mouse click simulation 