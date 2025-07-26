#include "joycursor_core.h"
#include "controller_manager.h"
#include "mapping_manager.h"
#include "config.h"
#include "../utils/logging.h"

JoyCursorCore::JoyCursorCore() 
    : m_controllerManager(std::unique_ptr<ControllerManager>(createControllerManager()))
    , m_deltaTime(0.005f) // Default to 5ms
    , m_lastPollTime(std::chrono::steady_clock::now()) {
}

JoyCursorCore::~JoyCursorCore() {
    shutdown();
}

bool JoyCursorCore::initialize() {
    try {
        // Initialize configuration
        m_config = std::make_unique<Config>();
        
        // Initialize mapping manager with the configuration
        m_mappingManager = std::make_unique<MappingManager>(m_config->getMappingsJson());
        
        // Connect controller manager callbacks to core events
        if (m_controllerManager) {
            m_controllerManager->setControllerConnectedCallback(
                [this](const std::string& guid, const std::string& name) {
                    onControllerConnected(guid, name);
                }
            );
            
            m_controllerManager->setControllerDisconnectedCallback(
                [this](const std::string& guid) {
                    onControllerDisconnected(guid);
                }
            );
        }
        
        // Initialize time tracking
        m_lastPollTime = std::chrono::steady_clock::now();
        m_deltaTime = 0.005f; // Start with 5ms
        
        logInfo("JoyCursorCore initialized successfully");
        return true;
    } catch (const std::exception& e) {
        logError(("Failed to initialize JoyCursorCore: " + std::string(e.what())).c_str());
        return false;
    }
}

void JoyCursorCore::shutdown() {
    if (m_controllerManager) {
        m_controllerManager.reset();
    }
    if (m_mappingManager) {
        m_mappingManager.reset();
    }
    if (m_config) {
        m_config.reset();
    }
}

void JoyCursorCore::pollEvents() {
    // Update delta time first
    updateDeltaTime();
    
    if (m_controllerManager) {
        m_controllerManager->pollEvents(m_deltaTime);
        processControllerEvents();
    }
}

void JoyCursorCore::updateDeltaTime() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastPollTime);
    m_deltaTime = elapsed.count() / 1000000.0f; // Convert to seconds
    
    // Clamp delta time to prevent huge jumps (e.g., if system was suspended)
    if (m_deltaTime > 0.1f) {
        m_deltaTime = 0.005f; // Cap at 100ms, default to 5ms
    }
    
    m_lastPollTime = now;
}

bool JoyCursorCore::hasActiveController() const {
    return m_controllerManager && m_controllerManager->hasActiveController();
}

std::string JoyCursorCore::getActiveControllerName() const {
    if (m_controllerManager) {
        return m_controllerManager->getActiveControllerName();
    }
    return "";
}

std::string JoyCursorCore::getActiveControllerGuid() const {
    // This would need to be implemented in the platform-specific controller manager
    // For now, return empty string
    return "";
}

std::map<std::string, std::string> JoyCursorCore::getKnownControllers() const {
    if (m_config) {
        return m_config->getKnownControllers();
    }
    return {};
}

std::map<std::string, std::string> JoyCursorCore::getConnectedControllers() const {
    return m_connectedControllers;
}

bool JoyCursorCore::loadConfiguration(const std::string& configPath) {
    try {
        if (m_config) {
            // Reload configuration
            m_config = std::make_unique<Config>();
            m_mappingManager = std::make_unique<MappingManager>(m_config->getMappingsJson());
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        logError(("Failed to load configuration: " + std::string(e.what())).c_str());
        return false;
    }
}

bool JoyCursorCore::saveConfiguration(const std::string& configPath) {
    try {
        if (m_config) {
            m_config->saveControllers();
            m_config->saveMappings();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        logError(("Failed to save configuration: " + std::string(e.what())).c_str());
        return false;
    }
}

void JoyCursorCore::clearMappingCache() {
    if (m_mappingManager) {
        m_mappingManager->clearCache();
    }
}

void JoyCursorCore::reloadControllerMappings() {
    if (m_controllerManager) {
        m_controllerManager->reloadMappings();
    }
}

StickMapping JoyCursorCore::getLeftStickMapping(const std::string& controllerGuid) {
    if (m_mappingManager) {
        return m_mappingManager->getLeftStick(controllerGuid);
    }
    return StickMapping{};
}

StickMapping JoyCursorCore::getRightStickMapping(const std::string& controllerGuid) {
    if (m_mappingManager) {
        return m_mappingManager->getRightStick(controllerGuid);
    }
    return StickMapping{};
}

ButtonMapping JoyCursorCore::getButtonMapping(const std::string& controllerGuid, const std::string& button) {
    if (m_mappingManager) {
        return m_mappingManager->getButtonMapping(controllerGuid, button);
    }
    return ButtonMapping{};
}

TriggerMapping JoyCursorCore::getTriggerMapping(const std::string& controllerGuid, const std::string& trigger) {
    if (m_mappingManager) {
        return m_mappingManager->getTriggerMapping(controllerGuid, trigger);
    }
    return TriggerMapping{};
}

void JoyCursorCore::setLeftStickMapping(const std::string& controllerGuid, const StickMapping& mapping) {
    if (m_mappingManager) {
        m_mappingManager->setLeftStickMapping(controllerGuid, mapping);
    }
}

void JoyCursorCore::setRightStickMapping(const std::string& controllerGuid, const StickMapping& mapping) {
    if (m_mappingManager) {
        m_mappingManager->setRightStickMapping(controllerGuid, mapping);
    }
}

void JoyCursorCore::setButtonMapping(const std::string& controllerGuid, const std::string& button, const ButtonMapping& mapping) {
    if (m_mappingManager) {
        m_mappingManager->setButtonMapping(controllerGuid, button, mapping);
    }
}

void JoyCursorCore::setTriggerMapping(const std::string& controllerGuid, const std::string& trigger, const TriggerMapping& mapping) {
    if (m_mappingManager) {
        m_mappingManager->setTriggerMapping(controllerGuid, trigger, mapping);
    }
}

void JoyCursorCore::addKnownController(const std::string& guid, const std::string& name) {
    if (m_config) {
        m_config->addController(guid, name);
    }
}

void JoyCursorCore::removeKnownController(const std::string& guid) {
    // This would need to be implemented in Config
    logInfo(("Removing known controller: " + guid).c_str());
}

void JoyCursorCore::setControllerConnectedCallback(ControllerConnectedCallback callback) {
    m_controllerConnectedCallback = callback;
}

void JoyCursorCore::setControllerDisconnectedCallback(ControllerDisconnectedCallback callback) {
    m_controllerDisconnectedCallback = callback;
}

void JoyCursorCore::setButtonEventCallback(ButtonEventCallback callback) {
    m_buttonEventCallback = callback;
}

void JoyCursorCore::setStickEventCallback(StickEventCallback callback) {
    m_stickEventCallback = callback;
}

void JoyCursorCore::setTriggerEventCallback(TriggerEventCallback callback) {
    m_triggerEventCallback = callback;
}

void JoyCursorCore::onControllerConnected(const std::string& guid, const std::string& name) {
    m_connectedControllers[guid] = name;
    if (m_controllerConnectedCallback) {
        m_controllerConnectedCallback(guid, name);
    }
}

void JoyCursorCore::onControllerDisconnected(const std::string& guid) {
    m_connectedControllers.erase(guid);
    if (m_controllerDisconnectedCallback) {
        m_controllerDisconnectedCallback(guid);
    }
}

void JoyCursorCore::processControllerEvents() {
    // Controller events are now handled directly by the controller manager callbacks
    // This method is kept for any additional event processing that might be needed
    // but the main connection/disconnection events are handled via callbacks
} 