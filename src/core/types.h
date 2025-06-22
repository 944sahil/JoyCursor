#pragma once

#include <vector>

// Enum for different stick action types
enum class StickActionType {
    NONE,       // No action assigned yet
    CURSOR,     // Move mouse cursor
    SCROLL      // Scroll mouse wheel
};

// Represents cursor movement action settings
struct CursorAction {
    float sensitivity = 0.05f;
    float boosted_sensitivity = 0.3f; // Used when L3/R3 is held
    float smoothing = 0.2f;
};

// Represents scroll action settings
struct ScrollAction {
    // Vertical scrolling (up/down)
    float vertical_sensitivity = 1.0f;
    int vertical_max_speed = 40;
    
    // Horizontal scrolling (left/right)
    float horizontal_sensitivity = 0.3f;
    int horizontal_max_speed = 15;
};

// Represents the mapping settings for stick control (left or right stick)
struct StickMapping {
    bool enabled = false;
    StickActionType action_type = StickActionType::NONE; // No action assigned by default
    int deadzone = 8000;
    
    // Action-specific settings - only the one matching action_type is used
    CursorAction cursor_action;
    ScrollAction scroll_action;
};

// Enum for different mouse click types
enum class MouseClickType {
    LEFT_CLICK,
    RIGHT_CLICK,
    MIDDLE_CLICK,
    NONE
};

// Enum for keyboard key types (Windows Virtual Key codes)
enum class KeyboardKeyType {
    NONE,
    // Arrow keys
    UP,
    DOWN,
    LEFT,
    RIGHT,
    // Common keys
    ENTER,
    ESCAPE,
    TAB,
    SPACE,
    // Modifier keys
    ALT,
    CTRL,
    SHIFT,
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
};

// Represents a single action that can be performed by a button
struct ButtonAction {
    MouseClickType click_type = MouseClickType::NONE;
    KeyboardKeyType key_type = KeyboardKeyType::NONE;
    bool enabled = false;
    
    // Keyboard-specific settings
    bool repeat_on_hold = false;     // Whether to repeat key when held
    int repeat_delay = 500;          // Milliseconds before repeat starts
    int repeat_interval = 100;       // Milliseconds between repeats
};

// Represents the mapping settings for a controller button
struct ButtonMapping {
    std::vector<ButtonAction> actions;
    bool enabled = false;
}; 