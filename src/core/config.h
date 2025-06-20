// config.h
// Handles loading and saving of JSON configuration files.

#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <set>

class Config {
public:
    Config(); // Constructor will load the files

    void saveControllers();
    void saveMappings();

    const std::set<std::string>& getKnownControllerGuids() const;
    void addControllerGuid(const std::string& guid);

    const nlohmann::json& getMappingsJson() const;
    nlohmann::json& getMappingsJson();


private:
    void loadControllers();
    void loadMappings();
    void createDefaultMappingsFile();

    nlohmann::json m_mappings;
    std::set<std::string> m_known_controller_guids;
}; 