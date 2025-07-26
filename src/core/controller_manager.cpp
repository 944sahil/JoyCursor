// controller_manager.cpp
// Platform-independent controller manager logic

#include "types.h"
#include "controller_manager.h"
#include "config.h"
#include "mapping_manager.h"
#include "utils/logging.h"
#include <nlohmann/json.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gamepad.h>
#include <fstream>
#include <set>
#include <string>
#include <unordered_map>
#include <cmath>

using nlohmann::json;

// Platform-specific function declarations
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

namespace {
const char* CONTROLLERS_JSON = "controllers.json";
const char* MAPPINGS_JSON = "mappings.json";

std::string guid_to_string(const SDL_GUID& guid) {
    char buf[64] = {0};
    SDL_GUIDToString(guid, buf, sizeof(buf));
    return std::string(buf);
}

json load_mappings() {
    std::ifstream in(MAPPINGS_JSON);
    json j;
    if (in) {
        in >> j;
    }
    return j;
}
}

class ControllerManagerImpl : public ControllerManager {
public:
    ControllerManagerImpl() : m_mapping_manager(m_config.getMappingsJson()) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS) < 0) {
            logError(SDL_GetError());
        } else {
            logInfo("SDL initialized for controller detection.");
        }
        
        // Initialize repeat timing tracking
        m_last_repeat_time = SDL_GetTicks();
    }

    ~ControllerManagerImpl() override {
        m_config.saveControllers();
        m_config.saveMappings();
        SDL_Quit();
    }

    void detectControllers() override {} // No-op for now

    void pollEvents(float deltaTime = 0.005f) override {
        SDL_UpdateGamepads();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_GAMEPAD_ADDED:
                    onGamepadAdded(event.gdevice);
                    break;
                case SDL_EVENT_GAMEPAD_REMOVED:
                    onGamepadRemoved(event.gdevice);
                    break;
                case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                    handleButtonDown(event.gbutton);
                    break;
                case SDL_EVENT_GAMEPAD_BUTTON_UP:
                    handleButtonUp(event.gbutton);
                    break;
            }
        }
        handleMouseMovement(deltaTime);
        handleTriggerButtons();
        handleTriggerScroll();
        handleRepeatTiming();
    }

    bool hasActiveController() const override {
        return !m_active_controllers.empty();
    }
    std::string getActiveControllerName() const override {
        if (!m_active_controllers.empty()) {
            auto it = m_active_controllers.begin();
            SDL_Gamepad* gamepad = it->second;
            const char* name = SDL_GetGamepadName(gamepad);
            return name ? std::string(name) : std::string();
        }
        return std::string();
    }

    // Callback setters for core integration
    void setControllerConnectedCallback(ControllerConnectedCallback callback) override {
        m_controllerConnectedCallback = callback;
    }
    
    void setControllerDisconnectedCallback(ControllerDisconnectedCallback callback) override {
        m_controllerDisconnectedCallback = callback;
    }

    void reloadMappings() {
        // Reload the config mappings from JSON file
        m_config.reloadMappings();
        
        // Clear the mapping manager's cache to force reload from JSON
        m_mapping_manager.clearCache();
        
        // Reload mappings for active controllers
        for (auto& [instance_id, gamepad] : m_active_controllers) {
            SDL_Joystick* joystick = SDL_GetGamepadJoystick(gamepad);
            SDL_GUID guid = SDL_GetJoystickGUID(joystick);
            std::string guid_str = guid_to_string(guid);
            
            // Reload mappings for this controller
            m_left_stick_mappings[instance_id] = m_mapping_manager.getLeftStick(guid_str);
            m_right_stick_mappings[instance_id] = m_mapping_manager.getRightStick(guid_str);
        }
        
        logInfo("Controller mappings reloaded from JSON");
    }

private:
    // Apply cubic scroll curve for natural feel
    float applyScrollCurve(float input) const {
        // Normalize input to [-1, 1] range first
        float normalized = input / 100.0f;
        // Apply cubic curve and preserve sign, but scale up for better responsiveness
        return std::copysign(normalized * normalized * normalized * 2.0f, normalized);
    }

    void onGamepadAdded(const SDL_GamepadDeviceEvent& event) {
        SDL_Gamepad* gamepad = SDL_OpenGamepad(event.which);
        if (!gamepad) {
            logError(SDL_GetError());
            return;
        }

        SDL_Joystick* joystick = SDL_GetGamepadJoystick(gamepad);
        SDL_GUID guid = SDL_GetJoystickGUID(joystick);
        std::string guid_str = guid_to_string(guid);
        const char* name = SDL_GetGamepadName(gamepad);

        m_active_controllers[event.which] = gamepad;
        m_left_stick_mappings[event.which] = m_mapping_manager.getLeftStick(guid_str);
        m_right_stick_mappings[event.which] = m_mapping_manager.getRightStick(guid_str);
        
        // Log the current mapping configuration
        const auto& left_mapping = m_left_stick_mappings[event.which];
        const auto& right_mapping = m_right_stick_mappings[event.which];
        
        std::string left_action_str, right_action_str;
        switch (left_mapping.action_type) {
            case StickActionType::CURSOR: left_action_str = "cursor"; break;
            case StickActionType::SCROLL: left_action_str = "scroll"; break;
            case StickActionType::NONE: left_action_str = "none"; break;
        }
        switch (right_mapping.action_type) {
            case StickActionType::CURSOR: right_action_str = "cursor"; break;
            case StickActionType::SCROLL: right_action_str = "scroll"; break;
            case StickActionType::NONE: right_action_str = "none"; break;
        }
        
        logInfo(("Mapping for controller [" + guid_str + "]: " +
            "left_stick=" + left_action_str + "(" + (left_mapping.enabled ? "enabled" : "disabled") + ")" +
            ", right_stick=" + right_action_str + "(" + (right_mapping.enabled ? "enabled" : "disabled") + ")").c_str());

        // Log all in-use button mappings for this controller
        std::vector<std::string> button_names = {
            "button_a", "button_b", "button_x", "button_y",
            "left_shoulder", "right_shoulder", "start", "back", "guide",
            "dpad_up", "dpad_down", "dpad_left", "dpad_right"
        };
        for (const auto& button_name : button_names) {
            ButtonMapping mapping = m_mapping_manager.getButtonMapping(guid_str, button_name);
            if (!mapping.enabled) continue;
            std::vector<std::string> enabled_actions;
            for (const auto& action : mapping.actions) {
                if (action.enabled) {
                    // Mouse click type
                    if (action.click_type != MouseClickType::NONE) {
                        switch (action.click_type) {
                            case MouseClickType::LEFT_CLICK: enabled_actions.push_back("mouse_left_click"); break;
                            case MouseClickType::RIGHT_CLICK: enabled_actions.push_back("mouse_right_click"); break;
                            case MouseClickType::MIDDLE_CLICK: enabled_actions.push_back("mouse_middle_click"); break;
                            default: break;
                        }
                    } else if (action.key_type != KeyboardKeyType::NONE) {
                        // Keyboard key type
                        switch (action.key_type) {
                            case KeyboardKeyType::ESCAPE: enabled_actions.push_back("keyboard_escape"); break;
                            case KeyboardKeyType::TAB: enabled_actions.push_back("keyboard_tab"); break;
                            case KeyboardKeyType::UP: enabled_actions.push_back("keyboard_up"); break;
                            case KeyboardKeyType::DOWN: enabled_actions.push_back("keyboard_down"); break;
                            case KeyboardKeyType::LEFT: enabled_actions.push_back("keyboard_left"); break;
                            case KeyboardKeyType::RIGHT: enabled_actions.push_back("keyboard_right"); break;
                            case KeyboardKeyType::ALT: enabled_actions.push_back("keyboard_alt"); break;
                            case KeyboardKeyType::CTRL: enabled_actions.push_back("keyboard_ctrl"); break;
                            case KeyboardKeyType::SHIFT: enabled_actions.push_back("keyboard_shift"); break;
                            case KeyboardKeyType::SPACE: enabled_actions.push_back("keyboard_space"); break;
                            case KeyboardKeyType::F1: enabled_actions.push_back("keyboard_f1"); break;
                            case KeyboardKeyType::F2: enabled_actions.push_back("keyboard_f2"); break;
                            case KeyboardKeyType::F3: enabled_actions.push_back("keyboard_f3"); break;
                            case KeyboardKeyType::F4: enabled_actions.push_back("keyboard_f4"); break;
                            case KeyboardKeyType::F5: enabled_actions.push_back("keyboard_f5"); break;
                            case KeyboardKeyType::F6: enabled_actions.push_back("keyboard_f6"); break;
                            case KeyboardKeyType::F7: enabled_actions.push_back("keyboard_f7"); break;
                            case KeyboardKeyType::F8: enabled_actions.push_back("keyboard_f8"); break;
                            case KeyboardKeyType::F9: enabled_actions.push_back("keyboard_f9"); break;
                            case KeyboardKeyType::F10: enabled_actions.push_back("keyboard_f10"); break;
                            case KeyboardKeyType::F11: enabled_actions.push_back("keyboard_f11"); break;
                            case KeyboardKeyType::F12: enabled_actions.push_back("keyboard_f12"); break;
                            default: break;
                        }
                    }
                }
            }
            if (!enabled_actions.empty()) {
                logInfo((button_name + ": " + [&](){ std::string s; for (const auto& a : enabled_actions) { if (!s.empty()) s += ", "; s += a; } return s; }()).c_str());
            }
        }
        // Log trigger mappings for this controller
        std::vector<std::string> trigger_names = {"left_trigger", "right_trigger"};
        for (const auto& trigger_name : trigger_names) {
            TriggerMapping trig_mapping = m_mapping_manager.getTriggerMapping(guid_str, trigger_name);
            if (!trig_mapping.enabled) continue;
            if (trig_mapping.action_type == TriggerActionType::SCROLL) {
                // logInfo((trigger_name + std::string(": scroll ") + trig_mapping.scroll_direction).c_str());
            } else if (trig_mapping.action_type == TriggerActionType::BUTTON) {
                // logInfo((trigger_name + std::string(": button action")).c_str());
            }
        }
        
        m_config.saveMappings();

        if (m_config.getKnownControllers().count(guid_str)) {
            logInfo(("Controller connected (known): " + std::string(name) + " [" + guid_str + "]").c_str());
        } else {
            logInfo(("Controller connected (new): " + std::string(name) + " [" + guid_str + "]").c_str());
            m_config.addController(guid_str, name ? std::string(name) : "Unknown Controller");
            m_config.saveControllers(); // Save immediately when new controller is added
        }
        
        // Notify core about controller connection
        if (m_controllerConnectedCallback) {
            m_controllerConnectedCallback(guid_str, name ? std::string(name) : "Unknown Controller");
        }
    }

    void onGamepadRemoved(const SDL_GamepadDeviceEvent& event) {
        if (m_active_controllers.count(event.which)) {
            const char* name = SDL_GetGamepadName(m_active_controllers[event.which]);
            
            // Get the GUID before closing the gamepad
            SDL_Joystick* joystick = SDL_GetGamepadJoystick(m_active_controllers[event.which]);
            SDL_GUID guid = SDL_GetJoystickGUID(joystick);
            std::string guid_str = guid_to_string(guid);
            
            logInfo(("Controller disconnected: " + std::string(name)).c_str());
            SDL_CloseGamepad(m_active_controllers[event.which]);
            m_active_controllers.erase(event.which);
            m_left_stick_mappings.erase(event.which);
            m_right_stick_mappings.erase(event.which);
            
            // Notify core about controller disconnection
            if (m_controllerDisconnectedCallback) {
                m_controllerDisconnectedCallback(guid_str);
            }
        }
    }

    void onGamepadAxis(const SDL_GamepadAxisEvent& event) {
        // Unused - polling instead
    }

    void handleMouseMovement(float deltaTime) {
        for (auto const& [instance_id, gamepad] : m_active_controllers) {
            float total_cursor_x = 0.0f;
            float total_cursor_y = 0.0f;
            bool has_cursor_movement = false;

            // Process left stick
            if (m_left_stick_mappings.count(instance_id) && m_left_stick_mappings.at(instance_id).enabled) {
                const auto& left_mapping = m_left_stick_mappings.at(instance_id);
                
                Sint16 left_x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
                Sint16 left_y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);

                if (std::abs(left_x) < left_mapping.deadzone) left_x = 0;
                if (std::abs(left_y) < left_mapping.deadzone) left_y = 0;
                
                float left_mx = std::clamp(static_cast<float>(left_x) / 32767.0f, -1.0f, 1.0f) * 100;
                float left_my = std::clamp(static_cast<float>(left_y) / 32767.0f, -1.0f, 1.0f) * 100;

                if (left_mapping.action_type == StickActionType::CURSOR) {
                    // Use boosted sensitivity if L3 is held (left stick button)
                    float effective_sensitivity = left_mapping.cursor_action.sensitivity;
                    if (m_l3_held[instance_id]) {
                        effective_sensitivity = left_mapping.cursor_action.boosted_sensitivity;
                    }

                    // Calculate movement per second (time-based)
                    float cursor_mx = left_mx * effective_sensitivity * 60.0f; // 60 pixels per second at full input
                    float cursor_my = left_my * effective_sensitivity * 60.0f;

                    // Smoothing logic with time-based movement
                    auto& vel = m_left_stick_velocity[instance_id];
                    vel.first = vel.first * (1.0f - left_mapping.cursor_action.smoothing) + cursor_mx * left_mapping.cursor_action.smoothing;
                    vel.second = vel.second * (1.0f - left_mapping.cursor_action.smoothing) + cursor_my * left_mapping.cursor_action.smoothing;

                    // Apply delta time to get movement for this frame
                    total_cursor_x += vel.first * deltaTime;
                    total_cursor_y += vel.second * deltaTime;
                    has_cursor_movement = true;
                } else if (left_mapping.action_type == StickActionType::SCROLL) {
                    // Multi-directional scroll mouse wheel
                    if (left_mx != 0.0f || left_my != 0.0f) {
                        // Get scroll settings
                        float vertical_sensitivity = left_mapping.scroll_action.vertical_sensitivity;
                        float horizontal_sensitivity = left_mapping.scroll_action.horizontal_sensitivity;
                        int vertical_max_speed = left_mapping.scroll_action.vertical_max_speed;
                        int horizontal_max_speed = left_mapping.scroll_action.horizontal_max_speed;
                        
                        // Calculate scroll amounts with cubic mapping for more natural feel
                        float curved_y = applyScrollCurve(left_my);
                        float curved_x = applyScrollCurve(left_mx);

                        // Optional: deadzone to avoid micro-scrolls (apply to original input)
                        if (std::abs(left_mx) < 5.0f) curved_x = 0; // 5% deadzone
                        if (std::abs(left_my) < 5.0f) curved_y = 0; // 5% deadzone

                        int scroll_y = static_cast<int>(-curved_y * vertical_sensitivity * vertical_max_speed);
                        int scroll_x = static_cast<int>(curved_x * horizontal_sensitivity * horizontal_max_speed);
                        
                        // Apply vertical scroll
                        if (scroll_y != 0) {
                            platform_simulate_scroll_vertical(scroll_y);
                        }
                        
                        // Apply horizontal scroll (if enabled)
                        if (scroll_x != 0) {
                            platform_simulate_scroll_horizontal(scroll_x);
                        }
                    }
                }
            }

            // Process right stick
            if (m_right_stick_mappings.count(instance_id) && m_right_stick_mappings.at(instance_id).enabled) {
                const auto& right_mapping = m_right_stick_mappings.at(instance_id);
                
                Sint16 right_x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
                Sint16 right_y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);

                if (std::abs(right_x) < right_mapping.deadzone) right_x = 0;
                if (std::abs(right_y) < right_mapping.deadzone) right_y = 0;
                
                float right_mx = std::clamp(static_cast<float>(right_x) / 32767.0f, -1.0f, 1.0f) * 100;
                float right_my = std::clamp(static_cast<float>(right_y) / 32767.0f, -1.0f, 1.0f) * 100;

                if (right_mapping.action_type == StickActionType::CURSOR) {
                    // Use boosted sensitivity if R3 is held (right stick button)
                    float effective_sensitivity = right_mapping.cursor_action.sensitivity;
                    if (m_r3_held[instance_id]) {
                        effective_sensitivity = right_mapping.cursor_action.boosted_sensitivity;
                    }

                    // Calculate movement per second (time-based)
                    float cursor_mx = right_mx * effective_sensitivity * 60.0f; // 60 pixels per second at full input
                    float cursor_my = right_my * effective_sensitivity * 60.0f;

                    // Smoothing logic with time-based movement
                    auto& vel = m_right_stick_velocity[instance_id];
                    vel.first = vel.first * (1.0f - right_mapping.cursor_action.smoothing) + cursor_mx * right_mapping.cursor_action.smoothing;
                    vel.second = vel.second * (1.0f - right_mapping.cursor_action.smoothing) + cursor_my * right_mapping.cursor_action.smoothing;

                    // Apply delta time to get movement for this frame
                    total_cursor_x += vel.first * deltaTime;
                    total_cursor_y += vel.second * deltaTime;
                    has_cursor_movement = true;
                } else if (right_mapping.action_type == StickActionType::SCROLL) {
                    // Multi-directional scroll mouse wheel
                    if (right_mx != 0.0f || right_my != 0.0f) {
                        // Get scroll settings
                        float vertical_sensitivity = right_mapping.scroll_action.vertical_sensitivity;
                        float horizontal_sensitivity = right_mapping.scroll_action.horizontal_sensitivity;
                        int vertical_max_speed = right_mapping.scroll_action.vertical_max_speed;
                        int horizontal_max_speed = right_mapping.scroll_action.horizontal_max_speed;
                        
                        // Calculate scroll amounts with cubic mapping for more natural feel
                        float curved_y = applyScrollCurve(right_my);
                        float curved_x = applyScrollCurve(right_mx);

                        // Optional: deadzone to avoid micro-scrolls (apply to original input)
                        if (std::abs(right_mx) < 5.0f) curved_x = 0; // 5% deadzone
                        if (std::abs(right_my) < 5.0f) curved_y = 0; // 5% deadzone

                        int scroll_y = static_cast<int>(-curved_y * vertical_sensitivity * vertical_max_speed);
                        int scroll_x = static_cast<int>(curved_x * horizontal_sensitivity * horizontal_max_speed);

                        // Apply vertical scroll
                        if (scroll_y != 0) {
                            platform_simulate_scroll_vertical(scroll_y);
                        }
                        
                        // Apply horizontal scroll (if enabled)
                        if (scroll_x != 0) {
                            platform_simulate_scroll_horizontal(scroll_x);
                        }
                    }
                }
            }

            // Apply combined cursor movement
            if (has_cursor_movement && (total_cursor_x != 0.0f || total_cursor_y != 0.0f)) {
                float current_x, current_y;
                SDL_GetGlobalMouseState(&current_x, &current_y);
                SDL_WarpMouseGlobal(current_x + total_cursor_x, current_y + total_cursor_y);
            }
        }
    }

    void handleTriggerButtons() {
        // Track previous pressed state for each controller and trigger
        static std::unordered_map<int, std::unordered_map<std::string, bool>> prev_trigger_pressed;
        for (const auto& [instance_id, gamepad] : m_active_controllers) {
            SDL_Joystick* joystick = SDL_GetGamepadJoystick(gamepad);
            SDL_GUID guid = SDL_GetJoystickGUID(joystick);
            std::string guid_str = guid_to_string(guid);

            struct TriggerInfo {
                SDL_GamepadAxis axis;
                const char* name;
            } triggers[2] = {
                {SDL_GAMEPAD_AXIS_LEFT_TRIGGER, "left_trigger"},
                {SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, "right_trigger"}
            };

            for (const auto& trig : triggers) {
                TriggerMapping mapping = m_mapping_manager.getTriggerMapping(guid_str, trig.name);
                if (!mapping.enabled || mapping.action_type != TriggerActionType::BUTTON) continue;

                Sint16 value = SDL_GetGamepadAxis(gamepad, trig.axis);
                bool pressed = value >= mapping.threshold;
                bool was_pressed = prev_trigger_pressed[instance_id][trig.name];

                if (pressed && !was_pressed) {
                    // Just pressed
                    executeButtonActionsDown(mapping.button_action, instance_id, trig.name);
                } else if (!pressed && was_pressed) {
                    // Just released
                    executeButtonActionsUp(mapping.button_action, instance_id, trig.name);
                }
                prev_trigger_pressed[instance_id][trig.name] = pressed;
            }
        }
    }

    void handleTriggerScroll() {
        static std::unordered_map<int, std::unordered_map<std::string, Uint32>> trigger_press_times;
        static Uint32 last_scroll_time = 0;
        Uint32 now = SDL_GetTicks();
        const Uint32 SCROLL_INTERVAL_MS = 10;
        const float BASE_SCROLL_PER_FRAME = 2.0f;
        const float MAX_SCROLL_PER_FRAME = 40.0f;
        const float MAX_ACCEL_TIME = 2000.0f; // ms

        if (now - last_scroll_time < SCROLL_INTERVAL_MS) return;
        last_scroll_time = now;

        for (const auto& [instance_id, gamepad] : m_active_controllers) {
            SDL_Joystick* joystick = SDL_GetGamepadJoystick(gamepad);
            SDL_GUID guid = SDL_GetJoystickGUID(joystick);
            std::string guid_str = guid_to_string(guid);

            struct TriggerInfo {
                SDL_GamepadAxis axis;
                const char* name;
                int direction; // +1 for up, -1 for down
            } triggers[2] = {
                {SDL_GAMEPAD_AXIS_LEFT_TRIGGER, "left_trigger", +1},
                {SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, "right_trigger", -1}
            };

            for (const auto& trig : triggers) {
                TriggerMapping mapping = m_mapping_manager.getTriggerMapping(guid_str, trig.name);
                if (!mapping.enabled) continue;

                if (mapping.action_type == TriggerActionType::SCROLL) {
                    // logInfo((std::string("Trigger: ") + trig.name + " mapped to scroll " + mapping.scroll_direction).c_str());
                } else if (mapping.action_type == TriggerActionType::BUTTON) {
                    // logInfo((std::string("Trigger: ") + trig.name + " mapped to button action").c_str());
                }

                Sint16 value = SDL_GetGamepadAxis(gamepad, trig.axis);
                if (value < mapping.threshold)
                {
                    trigger_press_times[instance_id][trig.name] = 0;
                    continue;
                }

                // Track held time
                Uint32& press_time = trigger_press_times[instance_id][trig.name];
                if (press_time == 0) press_time = now;
                float held_time = float(now - press_time);
                float accel = std::min(1.0f, held_time / MAX_ACCEL_TIME);
                float factor = accel * accel; // quadratic ramp-up

                // Normalize from threshold to max
                float norm = (value - mapping.threshold) / float(32767 - mapping.threshold);
                norm = std::clamp(norm, 0.0f, 1.0f);

                float base = mapping.trigger_scroll_action.vertical_sensitivity * BASE_SCROLL_PER_FRAME;
                float max = mapping.trigger_scroll_action.vertical_max_speed > 0 ? mapping.trigger_scroll_action.vertical_max_speed : MAX_SCROLL_PER_FRAME;
                int scroll_amount = static_cast<int>(base * norm * factor * max);
                if (scroll_amount == 0) continue;

                if (mapping.scroll_direction == "up") {
                    platform_simulate_scroll_vertical(scroll_amount);
                } else if (mapping.scroll_direction == "down") {
                    platform_simulate_scroll_vertical(-scroll_amount);
                }
            }
        }
    }

    void handleButtonDown(const SDL_GamepadButtonEvent& event) {
        // Handle L3 and R3 for boosted sensitivity
        if (event.button == SDL_GAMEPAD_BUTTON_LEFT_STICK) {
            m_l3_held[event.which] = true;
            return;
        }
        if (event.button == SDL_GAMEPAD_BUTTON_RIGHT_STICK) {
            m_r3_held[event.which] = true;
            return;
        }
        
        // Handle other buttons for actions
        std::string button_name = getButtonName(static_cast<SDL_GamepadButton>(event.button));
        if (!button_name.empty()) {
            // Track that this button is now held
            m_buttons_held[event.which].insert(button_name);
            executeButtonAction(event.which, button_name, true);
        }
    }

    void handleButtonUp(const SDL_GamepadButtonEvent& event) {
        // Handle L3 and R3 for boosted sensitivity
        if (event.button == SDL_GAMEPAD_BUTTON_LEFT_STICK) {
            m_l3_held[event.which] = false;
            return;
        }
        if (event.button == SDL_GAMEPAD_BUTTON_RIGHT_STICK) {
            m_r3_held[event.which] = false;
            return;
        }
        
        // Handle other buttons for actions
        std::string button_name = getButtonName(static_cast<SDL_GamepadButton>(event.button));
        if (!button_name.empty()) {
            // Remove from held buttons and execute release action
            m_buttons_held[event.which].erase(button_name);
            executeButtonAction(event.which, button_name, false);
        }
    }

    std::string getButtonName(SDL_GamepadButton button) {
        switch (button) {
            case SDL_GAMEPAD_BUTTON_SOUTH: return "button_a";
            case SDL_GAMEPAD_BUTTON_EAST: return "button_b";
            case SDL_GAMEPAD_BUTTON_WEST: return "button_x";
            case SDL_GAMEPAD_BUTTON_NORTH: return "button_y";
            case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return "left_shoulder";
            case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return "right_shoulder";
            case SDL_GAMEPAD_BUTTON_START: return "start";
            case SDL_GAMEPAD_BUTTON_BACK: return "back";
            case SDL_GAMEPAD_BUTTON_GUIDE: return "guide";
            case SDL_GAMEPAD_BUTTON_DPAD_UP: return "dpad_up";
            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return "dpad_down";
            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return "dpad_left";
            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return "dpad_right";
            default: return "";
        }
    }

    void executeButtonAction(int instance_id, const std::string& button_name, bool is_pressed) {
        if (!m_active_controllers.count(instance_id)) {
            return;
        }

        SDL_Joystick* joystick = SDL_GetGamepadJoystick(m_active_controllers[instance_id]);
        SDL_GUID guid = SDL_GetJoystickGUID(joystick);
        std::string guid_str = guid_to_string(guid);
        
        // Get button mapping and execute actions
        ButtonMapping mapping = m_mapping_manager.getButtonMapping(guid_str, button_name);
        if (mapping.enabled) {
            if (is_pressed) {
                executeButtonActionsDown(mapping, instance_id, button_name);
            } else {
                executeButtonActionsUp(mapping, instance_id, button_name);
            }
        }
    }

    void executeButtonActionsDown(const ButtonMapping& mapping, int instance_id, const std::string& button_name) {
        for (const auto& action : mapping.actions) {
            if (!action.enabled) {
                continue;
            }
            
            // Handle mouse clicks
            if (action.click_type != MouseClickType::NONE) {
                platform_simulate_mouse_down(static_cast<int>(action.click_type));
            }
            
            // Handle keyboard keys
            if (action.key_type != KeyboardKeyType::NONE) {
                platform_simulate_key_down(static_cast<int>(action.key_type));
                
                // Track press time for repeat logic
                if (action.repeat_on_hold) {
                    Uint32 current_time = SDL_GetTicks();
                    m_button_press_times[instance_id][button_name] = current_time;
                    m_last_repeat_times[instance_id][button_name] = current_time;
                }
            }
        }
    }

    void executeButtonActionsUp(const ButtonMapping& mapping, int instance_id, const std::string& button_name) {
        for (const auto& action : mapping.actions) {
            if (!action.enabled) {
                continue;
            }
            
            // Handle mouse clicks
            if (action.click_type != MouseClickType::NONE) {
                platform_simulate_mouse_up(static_cast<int>(action.click_type));
            }
            
            // Handle keyboard keys
            if (action.key_type != KeyboardKeyType::NONE) {
                platform_simulate_key_up(static_cast<int>(action.key_type));
                
                // Clear repeat timing tracking
                if (action.repeat_on_hold) {
                    m_button_press_times[instance_id].erase(button_name);
                    m_last_repeat_times[instance_id].erase(button_name);
                }
            }
        }
    }

    void handleRepeatTiming() {
        Uint32 current_time = SDL_GetTicks();
        
        for (auto const& [instance_id, held_buttons] : m_buttons_held) {
            for (const auto& button_name : held_buttons) {
                // Get the button mapping to check for repeat actions
                if (!m_active_controllers.count(instance_id)) {
                    continue;
                }
                
                SDL_Joystick* joystick = SDL_GetGamepadJoystick(m_active_controllers[instance_id]);
                SDL_GUID guid = SDL_GetJoystickGUID(joystick);
                std::string guid_str = guid_to_string(guid);
                
                ButtonMapping mapping = m_mapping_manager.getButtonMapping(guid_str, button_name);
                if (!mapping.enabled) {
                    continue;
                }
                
                for (const auto& action : mapping.actions) {
                    if (!action.enabled || action.key_type == KeyboardKeyType::NONE || !action.repeat_on_hold) {
                        continue;
                    }
                    
                    // Check if it's time for a repeat
                    auto& press_time = m_button_press_times[instance_id][button_name];
                    auto& last_repeat = m_last_repeat_times[instance_id][button_name];
                    
                    Uint32 time_since_press = current_time - press_time;
                    Uint32 time_since_last_repeat = current_time - last_repeat;
                    
                    // Initial delay before first repeat
                    if (time_since_press >= action.repeat_delay) {
                        // Check if it's time for the next repeat
                        if (time_since_last_repeat >= action.repeat_interval) {
                            platform_simulate_key_down(static_cast<int>(action.key_type));
                            platform_simulate_key_up(static_cast<int>(action.key_type));
                            last_repeat = current_time;
                        }
                    }
                }
            }
        }
    }

    Config m_config;
    MappingManager m_mapping_manager;
    std::unordered_map<int, SDL_Gamepad*> m_active_controllers;
    std::unordered_map<int, StickMapping> m_left_stick_mappings;
    std::unordered_map<int, StickMapping> m_right_stick_mappings;
    std::unordered_map<int, std::pair<float, float>> m_left_stick_velocity;
    std::unordered_map<int, std::pair<float, float>> m_right_stick_velocity;
    std::unordered_map<int, bool> m_l3_held;
    std::unordered_map<int, bool> m_r3_held;
    std::unordered_map<int, std::set<std::string>> m_buttons_held;
    
    // Repeat timing tracking
    Uint32 m_last_repeat_time;
    std::unordered_map<int, std::unordered_map<std::string, Uint32>> m_button_press_times;
    std::unordered_map<int, std::unordered_map<std::string, Uint32>> m_last_repeat_times;

    // Callback functions for core integration
    ControllerConnectedCallback m_controllerConnectedCallback = nullptr;
    ControllerDisconnectedCallback m_controllerDisconnectedCallback = nullptr;
};

// Factory function for main.cpp
ControllerManager* createControllerManager() {
    return new ControllerManagerImpl();
} 