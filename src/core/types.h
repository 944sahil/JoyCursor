#pragma once

// Represents the mapping settings for the left stick mouse control.
struct LeftStickMouseMapping {
    bool enabled = false;
    float sensitivity = 0.05f;
    int deadzone = 8000;
    float smoothing = 0.2f;
    float boosted_sensitivity = 0.3f; // Used when L3 is held
}; 