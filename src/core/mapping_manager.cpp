// mapping_manager.cpp
// Implementation for mapping manager

#include "mapping_manager.h"
#include "utils/logging.h"

// Platform-specific function declarations
extern "C" {
    void platform_simulate_mouse_click(int clickType);
    void platform_simulate_mouse_down(int clickType);
    void platform_simulate_mouse_up(int clickType);
}

MappingManager::MappingManager(nlohmann::json& mappings_json) 
    : m_mappings_json(mappings_json) {}

LeftStickMouseMapping MappingManager::getLeftStickMapping(const std::string& guid) {
    // Return cached mapping if already parsed
    if (m_parsed_mappings.count(guid)) {
        return m_parsed_mappings[guid];
    }

    // Check if a mapping for this GUID exists in the JSON
    if (!m_mappings_json["mappings"].contains(guid)) {
        createMappingFromDefault(guid);
    }

    // Parse the mapping from JSON
    LeftStickMouseMapping mapping;
    const auto& config = m_mappings_json["mappings"][guid]["left_stick_mouse"];
    mapping.enabled = config.value("enabled", false);
    mapping.sensitivity = config.value("sensitivity", 0.05f);
    mapping.deadzone = config.value("deadzone", 8000);
    mapping.smoothing = config.value("smoothing", 0.2f);
    mapping.boosted_sensitivity = config.value("boosted_sensitivity", 0.3f);

    // Cache and return
    m_parsed_mappings[guid] = mapping;
    return mapping;
}

ButtonMapping MappingManager::getButtonMapping(const std::string& guid, const std::string& button_name) {
    // Return cached mapping if already parsed
    if (m_parsed_button_mappings.count(guid) && m_parsed_button_mappings[guid].count(button_name)) {
        return m_parsed_button_mappings[guid][button_name];
    }

    // Check if a mapping for this GUID exists in the JSON
    if (!m_mappings_json["mappings"].contains(guid)) {
        createMappingFromDefault(guid);
    }

    // Parse the button mapping from JSON
    ButtonMapping mapping;
    const auto& buttons_config = m_mappings_json["mappings"][guid]["buttons"];
    
    if (buttons_config.contains(button_name)) {
        const auto& button_config = buttons_config[button_name];
        mapping.enabled = button_config.value("enabled", false);
        
        // Parse actions array
        if (button_config.contains("actions") && button_config["actions"].is_array()) {
            for (const auto& action_json : button_config["actions"]) {
                ButtonAction action;
                action.enabled = action_json.value("enabled", false);
                
                // Parse action_type string to click_type and key_type
                std::string action_type_str = action_json.value("action_type", "none");
                if (action_type_str == "mouse_left_click") {
                    action.click_type = MouseClickType::LEFT_CLICK;
                    action.key_type = KeyboardKeyType::NONE;
                } else if (action_type_str == "mouse_right_click") {
                    action.click_type = MouseClickType::RIGHT_CLICK;
                    action.key_type = KeyboardKeyType::NONE;
                } else if (action_type_str == "mouse_middle_click") {
                    action.click_type = MouseClickType::MIDDLE_CLICK;
                    action.key_type = KeyboardKeyType::NONE;
                } else if (action_type_str == "keyboard_space") {
                    action.click_type = MouseClickType::NONE;
                    action.key_type = KeyboardKeyType::NONE; // Future: Add SPACE to enum
                } else if (action_type_str == "keyboard_escape") {
                    action.click_type = MouseClickType::NONE;
                    action.key_type = KeyboardKeyType::NONE; // Future: Add ESCAPE to enum
                } else {
                    action.click_type = MouseClickType::NONE;
                    action.key_type = KeyboardKeyType::NONE;
                }
                
                mapping.actions.push_back(action);
            }
        }
    } else {
        // Button not found, create default disabled mapping
        mapping.enabled = false;
        ButtonAction default_action;
        default_action.click_type = MouseClickType::NONE;
        default_action.key_type = KeyboardKeyType::NONE;
        default_action.enabled = false;
        mapping.actions.push_back(default_action);
    }

    // Cache and return
    m_parsed_button_mappings[guid][button_name] = mapping;
    return mapping;
}

void MappingManager::executeButtonActions(const ButtonMapping& mapping) {
    if (!mapping.enabled) {
        return;
    }
    
    for (const auto& action : mapping.actions) {
        if (!action.enabled) {
            continue;
        }
        
        // Handle mouse clicks
        if (action.click_type != MouseClickType::NONE) {
            platform_simulate_mouse_click(static_cast<int>(action.click_type));
        }
        
        // Handle keyboard keys (TODO: implement when keyboard simulation is added)
        if (action.key_type != KeyboardKeyType::NONE) {
            logInfo("Keyboard action (not implemented yet)");
        }
    }
}

void MappingManager::createMappingFromDefault(const std::string& guid) {
    logInfo(("No mapping found for " + guid + ", creating from default profile.").c_str());
    if (m_mappings_json["mappings"].contains("default")) {
        m_mappings_json["mappings"][guid] = m_mappings_json["mappings"]["default"];
    } else {
        logError("Could not create new mapping, 'default' profile is missing in mappings.json!");
    }
} 