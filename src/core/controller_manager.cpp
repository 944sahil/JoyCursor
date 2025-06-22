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
}

namespace {
const char* CONTROLLERS_JSON = "controllers.json";
const char* MAPPINGS_JSON = "mappings.json";

std::set<std::string> load_known_guids() {
    std::set<std::string> guids;
    std::ifstream in(CONTROLLERS_JSON);
    if (in) {
        json j;
        in >> j;
        if (j.contains("controllers") && j["controllers"].is_array()) {
            for (const auto& guid : j["controllers"]) {
                guids.insert(guid.get<std::string>());
            }
        }
    }
    return guids;
}

void save_known_guids(const std::set<std::string>& guids) {
    json j;
    j["controllers"] = guids;
    std::ofstream out(CONTROLLERS_JSON);
    out << j.dump(4);
}

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
    }

    ~ControllerManagerImpl() override {
        m_config.saveControllers();
        m_config.saveMappings();
        SDL_Quit();
    }

    void detectControllers() override {} // No-op for now

    void pollEvents() override {
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
        handleMouseMovement();
    }

private:
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
        
        m_config.saveMappings();

        if (m_config.getKnownControllerGuids().count(guid_str)) {
            logInfo(("Controller connected (known): " + std::string(name) + " [" + guid_str + "]").c_str());
        } else {
            logInfo(("Controller connected (new): " + std::string(name) + " [" + guid_str + "]").c_str());
            m_config.addControllerGuid(guid_str);
        }
    }

    void onGamepadRemoved(const SDL_GamepadDeviceEvent& event) {
        if (m_active_controllers.count(event.which)) {
            const char* name = SDL_GetGamepadName(m_active_controllers[event.which]);
            logInfo(("Controller disconnected: " + std::string(name)).c_str());
            SDL_CloseGamepad(m_active_controllers[event.which]);
            m_active_controllers.erase(event.which);
            m_left_stick_mappings.erase(event.which);
            m_right_stick_mappings.erase(event.which);
        }
    }

    void onGamepadAxis(const SDL_GamepadAxisEvent& event) {
        // Unused - polling instead
    }

    void handleMouseMovement() {
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
                
                float left_mx = static_cast<float>(left_x) / 32767.0f * 100;
                float left_my = static_cast<float>(left_y) / 32767.0f * 100;

                if (left_mapping.action_type == StickActionType::CURSOR) {
                    // Use boosted sensitivity if L3 is held (left stick button)
                    float effective_sensitivity = left_mapping.cursor_action.sensitivity;
                    if (m_l3_held[instance_id]) {
                        effective_sensitivity = left_mapping.cursor_action.boosted_sensitivity;
                    }

                    float cursor_mx = left_mx * effective_sensitivity;
                    float cursor_my = left_my * effective_sensitivity;

                    // Smoothing logic
                    auto& vel = m_left_stick_velocity[instance_id];
                    vel.first = vel.first * (1.0f - left_mapping.cursor_action.smoothing) + cursor_mx * left_mapping.cursor_action.smoothing;
                    vel.second = vel.second * (1.0f - left_mapping.cursor_action.smoothing) + cursor_my * left_mapping.cursor_action.smoothing;

                    total_cursor_x += vel.first;
                    total_cursor_y += vel.second;
                    has_cursor_movement = true;
                } else if (left_mapping.action_type == StickActionType::SCROLL) {
                    // Scroll mouse wheel
                    if (left_mx != 0.0f || left_my != 0.0f) {
                        float scroll_x = left_mx * left_mapping.scroll_action.sensitivity;
                        float scroll_y = left_my * left_mapping.scroll_action.sensitivity;
                        
                        if (left_mapping.scroll_action.horizontal) {
                            // Horizontal scroll
                            if (scroll_x != 0.0f) {
                                // TODO: Implement horizontal scroll
                                logInfo(("Left stick horizontal scroll: " + std::to_string(scroll_x)).c_str());
                            }
                        } else {
                            // Vertical scroll
                            if (scroll_y != 0.0f) {
                                // TODO: Implement vertical scroll
                                logInfo(("Left stick vertical scroll: " + std::to_string(scroll_y)).c_str());
                            }
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
                
                float right_mx = static_cast<float>(right_x) / 32767.0f * 100;
                float right_my = static_cast<float>(right_y) / 32767.0f * 100;

                if (right_mapping.action_type == StickActionType::CURSOR) {
                    // Use boosted sensitivity if R3 is held (right stick button)
                    float effective_sensitivity = right_mapping.cursor_action.sensitivity;
                    if (m_r3_held[instance_id]) {
                        effective_sensitivity = right_mapping.cursor_action.boosted_sensitivity;
                    }

                    float cursor_mx = right_mx * effective_sensitivity;
                    float cursor_my = right_my * effective_sensitivity;

                    // Smoothing logic
                    auto& vel = m_right_stick_velocity[instance_id];
                    vel.first = vel.first * (1.0f - right_mapping.cursor_action.smoothing) + cursor_mx * right_mapping.cursor_action.smoothing;
                    vel.second = vel.second * (1.0f - right_mapping.cursor_action.smoothing) + cursor_my * right_mapping.cursor_action.smoothing;

                    total_cursor_x += vel.first;
                    total_cursor_y += vel.second;
                    has_cursor_movement = true;
                } else if (right_mapping.action_type == StickActionType::SCROLL) {
                    // Scroll mouse wheel
                    if (right_mx != 0.0f || right_my != 0.0f) {
                        float scroll_x = right_mx * right_mapping.scroll_action.sensitivity;
                        float scroll_y = right_my * right_mapping.scroll_action.sensitivity;
                        
                        if (right_mapping.scroll_action.horizontal) {
                            // Horizontal scroll
                            if (scroll_x != 0.0f) {
                                // TODO: Implement horizontal scroll
                                logInfo(("Right stick horizontal scroll: " + std::to_string(scroll_x)).c_str());
                            }
                        } else {
                            // Vertical scroll
                            if (scroll_y != 0.0f) {
                                // TODO: Implement vertical scroll
                                logInfo(("Right stick vertical scroll: " + std::to_string(scroll_y)).c_str());
                            }
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
                executeButtonActionsDown(mapping);
            } else {
                executeButtonActionsUp(mapping);
            }
        }
    }

    void executeButtonActionsDown(const ButtonMapping& mapping) {
        for (const auto& action : mapping.actions) {
            if (!action.enabled) {
                continue;
            }
            
            // Handle mouse clicks
            if (action.click_type != MouseClickType::NONE) {
                platform_simulate_mouse_down(static_cast<int>(action.click_type));
            }
            
            // Handle keyboard keys (TODO: implement when keyboard simulation is added)
            if (action.key_type != KeyboardKeyType::NONE) {
                logInfo("Keyboard action (not implemented yet)");
            }
        }
    }

    void executeButtonActionsUp(const ButtonMapping& mapping) {
        for (const auto& action : mapping.actions) {
            if (!action.enabled) {
                continue;
            }
            
            // Handle mouse clicks
            if (action.click_type != MouseClickType::NONE) {
                platform_simulate_mouse_up(static_cast<int>(action.click_type));
            }
            
            // Handle keyboard keys (TODO: implement when keyboard simulation is added)
            if (action.key_type != KeyboardKeyType::NONE) {
                logInfo("Keyboard action (not implemented yet)");
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
};

// Factory function for main.cpp
ControllerManager* createControllerManager() {
    return new ControllerManagerImpl();
} 