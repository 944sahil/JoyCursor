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

private:
    static INPUT createMouseInput(DWORD flags, DWORD data = 0);
};

// Platform-agnostic extern C interface for core layer
extern "C" {
    void platform_simulate_mouse_click(int clickType);
    void platform_simulate_mouse_down(int clickType);
    void platform_simulate_mouse_up(int clickType);
} 