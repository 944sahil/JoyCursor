// mapping_manager.h
// Parses and provides access to controller mapping configurations.

#pragma once

#include "types.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

// Represents the mapping settings for the left stick mouse control.

class MappingManager {
public:
    MappingManager(nlohmann::json& mappings_json);

    // Gets the left stick mapping for a given controller GUID.
    StickMapping getLeftStick(const std::string& guid);

    // Gets the right stick mapping for a given controller GUID.
    StickMapping getRightStick(const std::string& guid);

    // Gets the button mapping for a given controller GUID and button.
    // Creates a new mapping from default if the GUID is not found.
    ButtonMapping getButtonMapping(const std::string& guid, const std::string& button_name);

    // Executes all enabled actions for a button mapping
    static void executeButtonActions(const ButtonMapping& mapping);

    // Gets the trigger mapping for a given controller GUID and trigger (e.g., "left_trigger", "right_trigger").
    // Now also parses scroll_direction for scroll actions.
    TriggerMapping getTriggerMapping(const std::string& guid, const std::string& trigger_name);

    // --- ADDED: Setters for updating mappings ---
    void setButtonMapping(const std::string& guid, const std::string& button, const ButtonMapping& mapping);
    void setLeftStickMapping(const std::string& guid, const StickMapping& mapping);
    void setRightStickMapping(const std::string& guid, const StickMapping& mapping);
    void setTriggerMapping(const std::string& guid, const std::string& trigger, const TriggerMapping& mapping);

    // Clear cached mappings to force reload from JSON
    void clearCache();

private:
    void createMappingFromDefault(const std::string& guid);

    // Helper to parse a ButtonAction from JSON
    static ButtonAction parseButtonAction(const nlohmann::json& action_json);
    // Helper to parse a ButtonMapping from JSON
    static ButtonMapping parseButtonMapping(const nlohmann::json& button_config);

    nlohmann::json& m_mappings_json;
    std::unordered_map<std::string, StickMapping> m_parsed_left_stick_mappings;
    std::unordered_map<std::string, StickMapping> m_parsed_right_stick_mappings;
    std::unordered_map<std::string, std::unordered_map<std::string, ButtonMapping>> m_parsed_button_mappings;
    std::unordered_map<std::string, std::unordered_map<std::string, TriggerMapping>> m_parsed_trigger_mappings;
}; 