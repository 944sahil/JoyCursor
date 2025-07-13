// config.cpp
// Implementation for config 

#include "config.h"
#include "utils/logging.h"
#include <fstream>

namespace {
    const char* CONTROLLERS_JSON = "controllers.json";
    const char* MAPPINGS_JSON = "mappings.json";
    const char* RESOURCES_MAPPINGS = "mappings.json"; // Will be copied to build/bin/ by CMake
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
            for (const auto& controller : j["controllers"]) {
                if (controller.is_object() && controller.contains("guid") && controller.contains("name")) {
                    std::string guid = controller["guid"].get<std::string>();
                    std::string name = controller["name"].get<std::string>();
                    m_known_controllers[guid] = name;
                }
            }
        }
    }
}

void Config::saveControllers() {
    nlohmann::json j;
    nlohmann::json controllers_array = nlohmann::json::array();
    for (const auto& [guid, name] : m_known_controllers) {
        controllers_array.push_back({
            {"guid", guid},
            {"name", name}
        });
    }
    j["controllers"] = controllers_array;
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
    logInfo("mappings.json not found, creating from resources template.");
    
    // Try to read from resources (copied by CMake to build/bin/)
    std::ifstream resources_in(RESOURCES_MAPPINGS);
    if (resources_in) {
        try {
            nlohmann::json resources_mappings;
            resources_in >> resources_mappings;
            m_mappings = resources_mappings;
            logInfo("Successfully loaded mappings from resources template.");
        } catch (const std::exception& e) {
            logError("Failed to parse resources mappings.json, using fallback defaults.");
            createFallbackMappings();
        }
    } else {
        logInfo("Resources mappings.json not found, using fallback defaults.");
        createFallbackMappings();
    }
    
    saveMappings();
}

void Config::createFallbackMappings() {
    // Minimal fallback if resources file is missing or invalid
    m_mappings = {
        {"mappings", {
            {"default", {
                {"name", "Default Profile"},
                {"left_stick", {
                    {"enabled", true},
                    {"action_type", "cursor"},
                    {"deadzone", 8000},
                    {"cursor_action", {
                        {"sensitivity", 0.05},
                        {"boosted_sensitivity", 0.3},
                        {"smoothing", 0.2}
                    }},
                    {"scroll_action", {
                        {"vertical_sensitivity", 1.0},
                        {"horizontal_sensitivity", 0.5},
                        {"vertical_max_speed", 20},
                        {"horizontal_max_speed", 10}
                    }}
                }},
                {"right_stick", {
                    {"enabled", true},
                    {"action_type", "scroll"},
                    {"deadzone", 8000},
                    {"cursor_action", {
                        {"sensitivity", 0.3},
                        {"boosted_sensitivity", 0.05},
                        {"smoothing", 0.2}
                    }},
                    {"scroll_action", {
                        {"vertical_sensitivity", 1.0},
                        {"horizontal_sensitivity", 0.5},
                        {"vertical_max_speed", 20},
                        {"horizontal_max_speed", 10}
                    }}
                }},
                {"buttons", {
                    {"button_a", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "mouse_left_click"},
                                {"enabled", true}
                            }
                        }}
                    }},
                    {"button_b", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "keyboard_escape"},
                                {"enabled", true},
                                {"repeat_on_hold", false}
                            }
                        }}
                    }},
                    {"button_x", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "keyboard_enter"},
                                {"enabled", true},
                                {"repeat_on_hold", false}
                            }
                        }}
                    }},
                    {"button_y", {
                        {"enabled", false},
                        {"actions", {
                            {
                                {"action_type", "none"},
                                {"enabled", false}
                            }
                        }}
                    }},
                    {"left_shoulder", {
                        {"enabled", false},
                        {"actions", {
                            {
                                {"action_type", "none"},
                                {"enabled", false}
                            }
                        }}
                    }},
                    {"right_shoulder", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "mouse_right_click"},
                                {"enabled", true}
                            }
                        }}
                    }},
                    {"start", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "keyboard_tab"},
                                {"enabled", true},
                                {"repeat_on_hold", false}
                            }
                        }}
                    }},
                    {"back", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "keyboard_alt"},
                                {"enabled", true},
                                {"repeat_on_hold", false}
                            }
                        }}
                    }},
                    {"guide", {
                        {"enabled", false},
                        {"actions", {
                            {
                                {"action_type", "none"},
                                {"enabled", false}
                            }
                        }}
                    }},
                    {"dpad_up", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "keyboard_up"},
                                {"enabled", true},
                                {"repeat_on_hold", true},
                                {"repeat_delay", 500},
                                {"repeat_interval", 100}
                            }
                        }}
                    }},
                    {"dpad_down", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "keyboard_down"},
                                {"enabled", true},
                                {"repeat_on_hold", true},
                                {"repeat_delay", 500},
                                {"repeat_interval", 100}
                            }
                        }}
                    }},
                    {"dpad_left", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "keyboard_left"},
                                {"enabled", true},
                                {"repeat_on_hold", true},
                                {"repeat_delay", 500},
                                {"repeat_interval", 100}
                            }
                        }}
                    }},
                    {"dpad_right", {
                        {"enabled", true},
                        {"actions", {
                            {
                                {"action_type", "keyboard_right"},
                                {"enabled", true},
                                {"repeat_on_hold", true},
                                {"repeat_delay", 500},
                                {"repeat_interval", 100}
                            }
                        }}
                    }}
                }},
                {"triggers", {
                    {"left_trigger", {
                        {"enabled", true},
                        {"action_type", "scroll"},
                        {"threshold", 8000},
                        {"scroll_direction", "up"},
                        {"button_action", {
                            {"actions", {
                                {
                                    {"action_type", "none"},
                                    {"enabled", false}
                                }
                            }},
                            {"enabled", false}
                        }},
                        {"trigger_scroll_action", {
                            {"vertical_sensitivity", 1.0},
                            {"vertical_max_speed", 40}
                        }}
                    }},
                    {"right_trigger", {
                        {"enabled", true},
                        {"action_type", "scroll"},
                        {"threshold", 8000},
                        {"scroll_direction", "down"},
                        {"button_action", {
                            {"actions", {
                                {
                                    {"action_type", "none"},
                                    {"enabled", false}
                                }
                            }},
                            {"enabled", false}
                        }},
                        {"trigger_scroll_action", {
                            {"vertical_sensitivity", 1.0},
                            {"vertical_max_speed", 40}
                        }}
                    }}
                }}
            }}
        }}
    };
}

void Config::saveMappings() {
    std::ofstream out(MAPPINGS_JSON);
    out << m_mappings.dump(4);
}

const std::map<std::string, std::string>& Config::getKnownControllers() const {
    return m_known_controllers;
}

void Config::addController(const std::string& guid, const std::string& name) {
    m_known_controllers[guid] = name;
}

const nlohmann::json& Config::getMappingsJson() const {
    return m_mappings;
}

nlohmann::json& Config::getMappingsJson() {
    return m_mappings;
} 