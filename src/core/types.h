#pragma once

#include <vector>

// Represents the mapping settings for the left stick mouse control.
struct LeftStickMouseMapping {
    bool enabled = false;
    float sensitivity = 0.05f;
    int deadzone = 8000;
    float smoothing = 0.2f;
    float boosted_sensitivity = 0.3f; // Used when L3 is held
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