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
    float sensitivity = 1.0f;
    bool horizontal = false; // true for horizontal scroll, false for vertical
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

// Enum for keyboard key types (extensible for future)
enum class KeyboardKeyType {
    NONE
};

// Represents a single action that can be performed by a button
struct ButtonAction {
    MouseClickType click_type = MouseClickType::NONE;
    KeyboardKeyType key_type = KeyboardKeyType::NONE;
    bool enabled = false;
};

// Represents the mapping settings for a controller button
struct ButtonMapping {
    std::vector<ButtonAction> actions;
    bool enabled = false;
}; 