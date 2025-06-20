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
    // Creates a new mapping from default if the GUID is not found.
    LeftStickMouseMapping getLeftStickMapping(const std::string& guid);

private:
    void createMappingFromDefault(const std::string& guid);

    nlohmann::json& m_mappings_json;
    std::unordered_map<std::string, LeftStickMouseMapping> m_parsed_mappings;
}; 