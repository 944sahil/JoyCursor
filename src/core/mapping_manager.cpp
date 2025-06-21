// mapping_manager.cpp
// Implementation for mapping manager (to be implemented) 

#include "mapping_manager.h"
#include "utils/logging.h"

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

void MappingManager::createMappingFromDefault(const std::string& guid) {
    logInfo(("No mapping found for " + guid + ", creating from default profile.").c_str());
    if (m_mappings_json["mappings"].contains("default")) {
        m_mappings_json["mappings"][guid] = m_mappings_json["mappings"]["default"];
        // Save to disk after adding new mapping
        // This requires access to Config, so you may need to provide a callback or reference.
        // For now, let's call saveMappings() if possible.
    } else {
        logError("Could not create new mapping, 'default' profile is missing in mappings.json!");
    }
    // TODO: Save mappings to disk here
} 