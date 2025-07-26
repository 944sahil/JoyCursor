// config.h
// Handles loading and saving of JSON configuration files.

#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <map>

class Config {
public:
    Config(); // Constructor will load the files

    void saveControllers();
    void saveMappings();

    const std::map<std::string, std::string>& getKnownControllers() const;
    void addController(const std::string& guid, const std::string& name);

    const nlohmann::json& getMappingsJson() const;
    nlohmann::json& getMappingsJson();
    
    // Reload mappings from JSON file
    void reloadMappings();


private:
    void loadControllers();
    void loadMappings();
    void createDefaultMappingsFile();
    void createFallbackMappings();

    nlohmann::json m_mappings;
    std::map<std::string, std::string> m_known_controllers; // guid -> name
}; 