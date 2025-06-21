// controller_input_win.cpp
// Implementation for Windows controller input

#include "controller_input_win.h"
#include "../../utils/logging.h"

void ControllerInputWin::initialize() {
    // Windows-specific initialization if needed
}

void ControllerInputWin::pollInput() {
    // Windows-specific input polling if needed
}

INPUT ControllerInputWin::createMouseInput(DWORD flags, DWORD data) {
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = flags;
    input.mi.dwExtraInfo = data;
    return input;
}

void ControllerInputWin::simulateMouseClick(MouseClickType clickType) {
    simulateMouseDown(clickType);
    Sleep(1);
    simulateMouseUp(clickType);
}

void ControllerInputWin::simulateMouseDown(MouseClickType clickType) {
    INPUT input = {};
    
    switch (clickType) {
        case MouseClickType::LEFT_CLICK:
            input = createMouseInput(MOUSEEVENTF_LEFTDOWN);
            break;
            
        case MouseClickType::RIGHT_CLICK:
            input = createMouseInput(MOUSEEVENTF_RIGHTDOWN);
            break;
            
        case MouseClickType::MIDDLE_CLICK:
            input = createMouseInput(MOUSEEVENTF_MIDDLEDOWN);
            break;
            
        default:
            logError("Unknown mouse click type for mouse down");
            return;
    }
    
    SendInput(1, &input, sizeof(INPUT));
}

void ControllerInputWin::simulateMouseUp(MouseClickType clickType) {
    INPUT input = {};
    
    switch (clickType) {
        case MouseClickType::LEFT_CLICK:
            input = createMouseInput(MOUSEEVENTF_LEFTUP);
            break;
            
        case MouseClickType::RIGHT_CLICK:
            input = createMouseInput(MOUSEEVENTF_RIGHTUP);
            break;
            
        case MouseClickType::MIDDLE_CLICK:
            input = createMouseInput(MOUSEEVENTF_MIDDLEUP);
            break;
            
        default:
            logError("Unknown mouse click type for mouse up");
            return;
    }
    
    SendInput(1, &input, sizeof(INPUT));
}

// Platform-agnostic extern C interface implementation
extern "C" {
    void platform_simulate_mouse_click(int clickType) {
        ControllerInputWin::simulateMouseClick(static_cast<MouseClickType>(clickType));
    }
    
    void platform_simulate_mouse_down(int clickType) {
        ControllerInputWin::simulateMouseDown(static_cast<MouseClickType>(clickType));
    }
    
    void platform_simulate_mouse_up(int clickType) {
        ControllerInputWin::simulateMouseUp(static_cast<MouseClickType>(clickType));
    }
} 