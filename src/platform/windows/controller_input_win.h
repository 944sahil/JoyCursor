// controller_input_win.h
// Windows-specific controller input handling

#pragma once

#include <windows.h>
#include "../../core/types.h"

class ControllerInputWin {
public:
    static void initialize();
    static void pollInput();
    static void simulateMouseClick(MouseClickType clickType);
    static void simulateMouseDown(MouseClickType clickType);
    static void simulateMouseUp(MouseClickType clickType);
    
    // Keyboard simulation functions
    static void simulateKeyPress(KeyboardKeyType keyType);
    static void simulateKeyDown(KeyboardKeyType keyType);
    static void simulateKeyUp(KeyboardKeyType keyType);
    static WORD getVirtualKeyCode(KeyboardKeyType keyType);
    
    // Scroll simulation functions
    static void simulateScrollVertical(int amount);
    static void simulateScrollHorizontal(int amount);

private:
    static INPUT createMouseInput(DWORD flags, DWORD data = 0);
    static INPUT createKeyboardInput(WORD vkCode, DWORD flags = 0);
};

// Platform-agnostic extern C interface for core layer
extern "C" {
    void platform_simulate_mouse_click(int clickType);
    void platform_simulate_mouse_down(int clickType);
    void platform_simulate_mouse_up(int clickType);
    void platform_simulate_key_press(int keyType);
    void platform_simulate_key_down(int keyType);
    void platform_simulate_key_up(int keyType);
    void platform_simulate_scroll_vertical(int amount);
    void platform_simulate_scroll_horizontal(int amount);
} 