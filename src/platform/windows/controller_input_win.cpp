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

INPUT ControllerInputWin::createKeyboardInput(WORD vkCode, DWORD flags) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vkCode;
    input.ki.dwFlags = flags;
    return input;
}

void ControllerInputWin::simulateKeyPress(KeyboardKeyType keyType) {
    WORD vkCode = getVirtualKeyCode(keyType);
    if (vkCode != 0) {
        INPUT input = createKeyboardInput(vkCode);
        SendInput(1, &input, sizeof(INPUT));
        
        // Small delay to ensure the key press is registered
        Sleep(10);
        
        // Send key up
        input = createKeyboardInput(vkCode, KEYEVENTF_KEYUP);
        SendInput(1, &input, sizeof(INPUT));
    }
}

void ControllerInputWin::simulateKeyDown(KeyboardKeyType keyType) {
    WORD vkCode = getVirtualKeyCode(keyType);
    if (vkCode != 0) {
        INPUT input = createKeyboardInput(vkCode);
        SendInput(1, &input, sizeof(INPUT));
    }
}

void ControllerInputWin::simulateKeyUp(KeyboardKeyType keyType) {
    WORD vkCode = getVirtualKeyCode(keyType);
    if (vkCode != 0) {
        INPUT input = createKeyboardInput(vkCode, KEYEVENTF_KEYUP);
        SendInput(1, &input, sizeof(INPUT));
    }
}

WORD ControllerInputWin::getVirtualKeyCode(KeyboardKeyType keyType) {
    switch (keyType) {
        // Arrow keys
        case KeyboardKeyType::UP: return VK_UP;
        case KeyboardKeyType::DOWN: return VK_DOWN;
        case KeyboardKeyType::LEFT: return VK_LEFT;
        case KeyboardKeyType::RIGHT: return VK_RIGHT;
        
        // Common keys
        case KeyboardKeyType::ENTER: return VK_RETURN;
        case KeyboardKeyType::ESCAPE: return VK_ESCAPE;
        case KeyboardKeyType::TAB: return VK_TAB;
        case KeyboardKeyType::SPACE: return VK_SPACE;
        
        // Modifier keys
        case KeyboardKeyType::ALT: return VK_MENU;
        case KeyboardKeyType::CTRL: return VK_CONTROL;
        case KeyboardKeyType::SHIFT: return VK_SHIFT;
        
        // Function keys
        case KeyboardKeyType::F1: return VK_F1;
        case KeyboardKeyType::F2: return VK_F2;
        case KeyboardKeyType::F3: return VK_F3;
        case KeyboardKeyType::F4: return VK_F4;
        case KeyboardKeyType::F5: return VK_F5;
        case KeyboardKeyType::F6: return VK_F6;
        case KeyboardKeyType::F7: return VK_F7;
        case KeyboardKeyType::F8: return VK_F8;
        case KeyboardKeyType::F9: return VK_F9;
        case KeyboardKeyType::F10: return VK_F10;
        case KeyboardKeyType::F11: return VK_F11;
        case KeyboardKeyType::F12: return VK_F12;
        
        default: return 0;
    }
}

// Platform-agnostic extern C interface implementations
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
    
    void platform_simulate_key_press(int keyType) {
        ControllerInputWin::simulateKeyPress(static_cast<KeyboardKeyType>(keyType));
    }
    
    void platform_simulate_key_down(int keyType) {
        ControllerInputWin::simulateKeyDown(static_cast<KeyboardKeyType>(keyType));
    }
    
    void platform_simulate_key_up(int keyType) {
        ControllerInputWin::simulateKeyUp(static_cast<KeyboardKeyType>(keyType));
    }
} 