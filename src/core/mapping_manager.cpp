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

StickMapping MappingManager::getLeftStick(const std::string& guid) {
    // Return cached mapping if already parsed
    if (m_parsed_left_stick_mappings.count(guid)) {
        return m_parsed_left_stick_mappings[guid];
    }

    // Check if a mapping for this GUID exists in the JSON
    if (!m_mappings_json["mappings"].contains(guid)) {
        createMappingFromDefault(guid);
    }

    // Parse the mapping from JSON
    StickMapping mapping;
    const auto& config = m_mappings_json["mappings"][guid]["left_stick"];
    mapping.enabled = config.value("enabled", false);
    
    // Parse action_type
    std::string action_type_str = config.value("action_type", "");
    if (action_type_str == "scroll") {
        mapping.action_type = StickActionType::SCROLL;
    } else if (action_type_str == "cursor") {
        mapping.action_type = StickActionType::CURSOR;
    } else {
        mapping.action_type = StickActionType::NONE;
    }
    
    mapping.deadzone = config.value("deadzone", mapping.deadzone);
    
    // Parse cursor action settings
    if (config.contains("cursor_action")) {
        const auto& cursor_config = config["cursor_action"];
        mapping.cursor_action.sensitivity = cursor_config.value("sensitivity", mapping.cursor_action.sensitivity);
        mapping.cursor_action.boosted_sensitivity = cursor_config.value("boosted_sensitivity", mapping.cursor_action.boosted_sensitivity);
        mapping.cursor_action.smoothing = cursor_config.value("smoothing", mapping.cursor_action.smoothing);
    }
    
    // Parse scroll action settings
    if (config.contains("scroll_action")) {
        const auto& scroll_config = config["scroll_action"];
        
        // Parse multi-directional scroll settings
        mapping.scroll_action.vertical_sensitivity = scroll_config.value("vertical_sensitivity", mapping.scroll_action.vertical_sensitivity);
        mapping.scroll_action.horizontal_sensitivity = scroll_config.value("horizontal_sensitivity", mapping.scroll_action.horizontal_sensitivity);
        mapping.scroll_action.vertical_max_speed = scroll_config.value("vertical_max_speed", mapping.scroll_action.vertical_max_speed);
        mapping.scroll_action.horizontal_max_speed = scroll_config.value("horizontal_max_speed", mapping.scroll_action.horizontal_max_speed);
    }

    // Cache and return
    m_parsed_left_stick_mappings[guid] = mapping;
    return mapping;
}

StickMapping MappingManager::getRightStick(const std::string& guid) {
    // Return cached mapping if already parsed
    if (m_parsed_right_stick_mappings.count(guid)) {
        return m_parsed_right_stick_mappings[guid];
    }

    // Check if a mapping for this GUID exists in the JSON
    if (!m_mappings_json["mappings"].contains(guid)) {
        createMappingFromDefault(guid);
    }

    // Parse the mapping from JSON
    StickMapping mapping;
    const auto& config = m_mappings_json["mappings"][guid]["right_stick"];
    mapping.enabled = config.value("enabled", false);
    
    // Parse action_type
    std::string action_type_str = config.value("action_type", "");
    if (action_type_str == "scroll") {
        mapping.action_type = StickActionType::SCROLL;
    } else if (action_type_str == "cursor") {
        mapping.action_type = StickActionType::CURSOR;
    } else {
        mapping.action_type = StickActionType::NONE;
    }
    
    mapping.deadzone = config.value("deadzone", mapping.deadzone);
    
    // Parse cursor action settings
    if (config.contains("cursor_action")) {
        const auto& cursor_config = config["cursor_action"];
        mapping.cursor_action.sensitivity = cursor_config.value("sensitivity", mapping.cursor_action.sensitivity);
        mapping.cursor_action.boosted_sensitivity = cursor_config.value("boosted_sensitivity", mapping.cursor_action.boosted_sensitivity);
        mapping.cursor_action.smoothing = cursor_config.value("smoothing", mapping.cursor_action.smoothing);
    }
    
    // Parse scroll action settings
    if (config.contains("scroll_action")) {
        const auto& scroll_config = config["scroll_action"];
        
        // Parse multi-directional scroll settings
        mapping.scroll_action.vertical_sensitivity = scroll_config.value("vertical_sensitivity", mapping.scroll_action.vertical_sensitivity);
        mapping.scroll_action.horizontal_sensitivity = scroll_config.value("horizontal_sensitivity", mapping.scroll_action.horizontal_sensitivity);
        mapping.scroll_action.vertical_max_speed = scroll_config.value("vertical_max_speed", mapping.scroll_action.vertical_max_speed);
        mapping.scroll_action.horizontal_max_speed = scroll_config.value("horizontal_max_speed", mapping.scroll_action.horizontal_max_speed);
    }

    // Cache and return
    m_parsed_right_stick_mappings[guid] = mapping;
    return mapping;
}

ButtonAction MappingManager::parseButtonAction(const nlohmann::json& action_json) {
    ButtonAction action;
    action.enabled = action_json.value("enabled", false);
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
        action.key_type = KeyboardKeyType::SPACE;
    } else if (action_type_str == "keyboard_escape") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::ESCAPE;
    } else if (action_type_str == "keyboard_enter") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::ENTER;
    } else if (action_type_str == "keyboard_tab") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::TAB;
    } else if (action_type_str == "keyboard_alt") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::ALT;
    } else if (action_type_str == "keyboard_ctrl") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::CTRL;
    } else if (action_type_str == "keyboard_shift") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::SHIFT;
    } else if (action_type_str == "keyboard_up") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::UP;
    } else if (action_type_str == "keyboard_down") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::DOWN;
    } else if (action_type_str == "keyboard_left") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::LEFT;
    } else if (action_type_str == "keyboard_right") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::RIGHT;
    } else if (action_type_str == "keyboard_f1") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F1;
    } else if (action_type_str == "keyboard_f2") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F2;
    } else if (action_type_str == "keyboard_f3") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F3;
    } else if (action_type_str == "keyboard_f4") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F4;
    } else if (action_type_str == "keyboard_f5") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F5;
    } else if (action_type_str == "keyboard_f6") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F6;
    } else if (action_type_str == "keyboard_f7") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F7;
    } else if (action_type_str == "keyboard_f8") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F8;
    } else if (action_type_str == "keyboard_f9") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F9;
    } else if (action_type_str == "keyboard_f10") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F10;
    } else if (action_type_str == "keyboard_f11") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F11;
    } else if (action_type_str == "keyboard_f12") {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::F12;
    } else {
        action.click_type = MouseClickType::NONE;
        action.key_type = KeyboardKeyType::NONE;
    }
    action.repeat_on_hold = action_json.value("repeat_on_hold", action.repeat_on_hold);
    action.repeat_delay = action_json.value("repeat_delay", action.repeat_delay);
    action.repeat_interval = action_json.value("repeat_interval", action.repeat_interval);
    return action;
}

ButtonMapping MappingManager::parseButtonMapping(const nlohmann::json& button_config) {
    ButtonMapping mapping;
    mapping.enabled = button_config.value("enabled", false);
    if (button_config.contains("actions") && button_config["actions"].is_array()) {
        for (const auto& action_json : button_config["actions"]) {
            mapping.actions.push_back(parseButtonAction(action_json));
        }
    }
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

    ButtonMapping mapping;
    const auto& buttons_config = m_mappings_json["mappings"][guid]["buttons"];
    if (buttons_config.contains(button_name)) {
        mapping = parseButtonMapping(buttons_config[button_name]);
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

TriggerMapping MappingManager::getTriggerMapping(const std::string& guid, const std::string& trigger_name) {
    if (m_parsed_trigger_mappings.count(guid) && m_parsed_trigger_mappings[guid].count(trigger_name)) {
        return m_parsed_trigger_mappings[guid][trigger_name];
    }
    if (!m_mappings_json["mappings"].contains(guid)) {
        createMappingFromDefault(guid);
    }
    TriggerMapping mapping;
    const auto& triggers_config = m_mappings_json["mappings"][guid]["triggers"];
    if (triggers_config.contains(trigger_name)) {
        const auto& trigger_config = triggers_config[trigger_name];
        mapping.enabled = trigger_config.value("enabled", false);
        std::string action_type_str = trigger_config.value("action_type", "none");
        if (action_type_str == "button") {
            mapping.action_type = TriggerActionType::BUTTON;
        } else if (action_type_str == "scroll") {
            mapping.action_type = TriggerActionType::SCROLL;
        } else {
            mapping.action_type = TriggerActionType::NONE;
        }
        mapping.threshold = trigger_config.value("threshold", mapping.threshold);
        // Parse button_action using helper
        if (trigger_config.contains("button_action")) {
            mapping.button_action = parseButtonMapping(trigger_config["button_action"]);
        }
        // (Skip scroll_action for now)
    } else {
        mapping.enabled = false;
        mapping.action_type = TriggerActionType::NONE;
        mapping.threshold = 8000;
        mapping.button_action.enabled = false;
        mapping.button_action.actions.clear();
    }
    m_parsed_trigger_mappings[guid][trigger_name] = mapping;
    return mapping;
} 