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
        m_active_mappings[event.which] = m_mapping_manager.getLeftStickMapping(guid_str);
        
        // Log the current mapping configuration
        const auto& mapping = m_active_mappings[event.which];
        logInfo(("Mapping for controller [" + guid_str + "]: " +
            "enabled=" + (mapping.enabled ? "true" : "false") +
            ", sensitivity=" + std::to_string(mapping.sensitivity) +
            ", boosted_sensitivity=" + std::to_string(mapping.boosted_sensitivity) +
            ", deadzone=" + std::to_string(mapping.deadzone) +
            ", smoothing=" + std::to_string(mapping.smoothing)).c_str());
        
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
            m_active_mappings.erase(event.which);
        }
    }

    void onGamepadAxis(const SDL_GamepadAxisEvent& event) {
        // Unused - polling instead
    }

    void handleMouseMovement() {
        for (auto const& [instance_id, gamepad] : m_active_controllers) {
            if (!m_active_mappings.count(instance_id) || !m_active_mappings.at(instance_id).enabled) {
                continue;
            }

            const auto& mapping = m_active_mappings.at(instance_id);

            // Use boosted sensitivity if L3 is held
            float effective_sensitivity = mapping.sensitivity;
            if (m_l3_held[instance_id]) {
                effective_sensitivity = mapping.boosted_sensitivity;
            }

            Sint16 x_axis = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
            Sint16 y_axis = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);

            if (std::abs(x_axis) < mapping.deadzone) x_axis = 0;
            if (std::abs(y_axis) < mapping.deadzone) y_axis = 0;
            
            float mx = static_cast<float>(x_axis) / 32767.0f * effective_sensitivity * 100;
            float my = static_cast<float>(y_axis) / 32767.0f * effective_sensitivity * 100;

            // Smoothing logic
            auto& vel = m_left_stick_velocity[instance_id];
            vel.first = vel.first * (1.0f - mapping.smoothing) + mx * mapping.smoothing;
            vel.second = vel.second * (1.0f - mapping.smoothing) + my * mapping.smoothing;

            // Only move if velocity is nonzero
            if (vel.first != 0.0f || vel.second != 0.0f) {
                float current_x, current_y;
                SDL_GetGlobalMouseState(&current_x, &current_y);
                SDL_WarpMouseGlobal(current_x + vel.first, current_y + vel.second);
            }
        }
    }

    void handleButtonDown(const SDL_GamepadButtonEvent& event) {
        // Handle L3 for boosted sensitivity
        if (event.button == SDL_GAMEPAD_BUTTON_LEFT_STICK) {
            m_l3_held[event.which] = true;
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
        // Handle L3 for boosted sensitivity
        if (event.button == SDL_GAMEPAD_BUTTON_LEFT_STICK) {
            m_l3_held[event.which] = false;
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
        
        // Get the controller's GUID
        SDL_Gamepad* gamepad = m_active_controllers[instance_id];
        SDL_Joystick* joystick = SDL_GetGamepadJoystick(gamepad);
        SDL_GUID guid = SDL_GetJoystickGUID(joystick);
        std::string guid_str = guid_to_string(guid);
        
        // Get button mapping and execute actions
        ButtonMapping mapping = m_mapping_manager.getButtonMapping(guid_str, button_name);
        if (mapping.enabled) {
            if (is_pressed) {
                // Button pressed - execute mouse down actions
                executeButtonActionsDown(mapping);
            } else {
                // Button released - execute mouse up actions
                executeButtonActionsUp(mapping);
            }
        }
    }

    void executeButtonActionsDown(const ButtonMapping& mapping) {
        for (const auto& action : mapping.actions) {
            if (!action.enabled) {
                continue;
            }
            
            // Handle mouse clicks - send down event
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
            
            // Handle mouse clicks - send up event
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
    std::unordered_map<int, LeftStickMouseMapping> m_active_mappings;
    std::unordered_map<int, std::pair<float, float>> m_left_stick_velocity;
    std::unordered_map<int, bool> m_l3_held;
    std::unordered_map<int, std::set<std::string>> m_buttons_held;
};

// Factory function for main.cpp
ControllerManager* createControllerManager() {
    return new ControllerManagerImpl();
} 