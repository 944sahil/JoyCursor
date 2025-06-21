# JoyCursor

A C++ application that maps game controller inputs to mouse actions, allowing you to control your computer using a gamepad.

## Features

- **Controller-to-Mouse Mapping**: Map controller buttons to mouse clicks
- **Analog Stick Control**: Use the left analog stick for mouse movement
- **JSON Configuration**: Flexible mapping through JSON files
- **Auto-Detection**: Automatically detects and remembers controllers
- **Windows Support**: Mouse simulation for Windows

## Requirements

- **C++17** compatible compiler
- **CMake** 3.16 or higher
- **SDL3** library
- **Windows**

## Building

```bash
# Clone the repository
git clone https://github.com/944sahil/JoyCursor.git
cd JoyCursor

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .
```

The executable will be created in `build/bin/JoyCursor.exe`.

## Usage

### Running

```bash
cd build/bin
./JoyCursor.exe
```

### Default Controls

- **Left Analog Stick**: Mouse movement
- **A Button**: Left mouse click
- **Right Shoulder**: Right mouse click
- **L3 (Left Stick Button)**: Boost mouse sensitivity when held

### Configuration

The application automatically creates configuration files:

- `mappings.json`: Button and stick mapping configurations
- `controllers.json`: List of known controller GUIDs

#### Customizing Mappings

Edit `mappings.json` to customize your controller mappings:

```json
{
  "mappings": {
    "default": {
      "left_stick_mouse": {
        "enabled": true,
        "sensitivity": 0.05,
        "deadzone": 8000,
        "smoothing": 0.2,
        "boosted_sensitivity": 0.3
      },
      "buttons": {
        "button_a": {
          "enabled": true,
          "actions": [
            {
              "action_type": "mouse_left_click",
              "enabled": true
            }
          ]
        }
      }
    }
  }
}
```

#### Supported Actions

- `mouse_left_click`: Left mouse button
- `mouse_right_click`: Right mouse button
- `mouse_middle_click`: Middle mouse button

## Project Structure

```
JoyCursor/
├── src/
│   ├── core/           # Core application logic
│   ├── platform/       # Platform-specific implementations
│   ├── resources/      # Configuration templates
│   └── utils/          # Utility functions
├── experiments/        # Experimental prototypes
└── CMakeLists.txt     # Build configuration
```

## Development

The `experiments/` directory contains prototypes and experimental features for reference.
