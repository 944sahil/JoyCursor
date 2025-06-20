// config.cpp
// Implementation for config (to be implemented) 

#include "config.h"
#include "utils/logging.h"
#include <fstream>

namespace {
    const char* CONTROLLERS_JSON = "controllers.json";
    const char* MAPPINGS_JSON = "mappings.json";
}

Config::Config() {
    loadControllers();
    loadMappings();
}

void Config::loadControllers() {
    std::ifstream in(CONTROLLERS_JSON);
    if (in) {
        nlohmann::json j;
        in >> j;
        if (j.contains("controllers") && j["controllers"].is_array()) {
            for (const auto& guid : j["controllers"]) {
                m_known_controller_guids.insert(guid.get<std::string>());
            }
        }
    }
}

void Config::saveControllers() {
    nlohmann::json j;
    j["controllers"] = m_known_controller_guids;
    std::ofstream out(CONTROLLERS_JSON);
    out << j.dump(4);
}

void Config::loadMappings() {
    std::ifstream in(MAPPINGS_JSON);
    if (in) {
        in >> m_mappings;
    } else {
        createDefaultMappingsFile();
        // Try loading again after creation
        std::ifstream created_in(MAPPINGS_JSON);
        if (created_in) {
            created_in >> m_mappings;
        }
    }
}

void Config::createDefaultMappingsFile() {
    logInfo("mappings.json not found, creating a default one.");
    m_mappings = {
        {"mappings", {
            {"default", {
                {"name", "Default Profile"},
                {"left_stick_mouse", {
                    {"enabled", true},
                    {"sensitivity", 0.05},
                    {"deadzone", 8000},
                    {"smoothing", 0.2}
                }}
            }}
        }}
    };
    saveMappings();
}

void Config::saveMappings() {
    std::ofstream out(MAPPINGS_JSON);
    out << m_mappings.dump(4);
}

const std::set<std::string>& Config::getKnownControllerGuids() const {
    return m_known_controller_guids;
}

void Config::addControllerGuid(const std::string& guid) {
    m_known_controller_guids.insert(guid);
}

const nlohmann::json& Config::getMappingsJson() const {
    return m_mappings;
}

nlohmann::json& Config::getMappingsJson() {
    return m_mappings;
} 