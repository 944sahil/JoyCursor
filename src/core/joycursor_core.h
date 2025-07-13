#pragma once

#include "types.h"
#include <string>
#include <functional>
#include <memory>
#include <map>
#include <chrono>

// Forward declarations
class ControllerManager;
class MappingManager;
class Config;

// Callback types for GUI integration
using ControllerConnectedCallback = std::function<void(const std::string& guid, const std::string& name)>;
using ControllerDisconnectedCallback = std::function<void(const std::string& guid)>;
using ButtonEventCallback = std::function<void(const std::string& guid, const std::string& button, bool pressed)>;
using StickEventCallback = std::function<void(const std::string& guid, const std::string& stick, float x, float y)>;
using TriggerEventCallback = std::function<void(const std::string& guid, const std::string& trigger, float value)>;

// Main core class that unifies all functionality
class JoyCursorCore {
public:
    JoyCursorCore();
    ~JoyCursorCore();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void pollEvents();

    // Controller management
    bool hasActiveController() const;
    std::string getActiveControllerName() const;
    std::string getActiveControllerGuid() const;
    
    // Get all known controllers
    std::map<std::string, std::string> getKnownControllers() const; // guid -> name
    std::map<std::string, std::string> getConnectedControllers() const; // guid -> name
    
    // Configuration management
    bool loadConfiguration(const std::string& configPath = "");
    bool saveConfiguration(const std::string& configPath = "");
    
    // Mapping access
    StickMapping getLeftStickMapping(const std::string& controllerGuid);
    StickMapping getRightStickMapping(const std::string& controllerGuid);
    ButtonMapping getButtonMapping(const std::string& controllerGuid, const std::string& button);
    TriggerMapping getTriggerMapping(const std::string& controllerGuid, const std::string& trigger);
    
    // Mapping modification
    void setLeftStickMapping(const std::string& controllerGuid, const StickMapping& mapping);
    void setRightStickMapping(const std::string& controllerGuid, const StickMapping& mapping);
    void setButtonMapping(const std::string& controllerGuid, const std::string& button, const ButtonMapping& mapping);
    void setTriggerMapping(const std::string& controllerGuid, const std::string& trigger, const TriggerMapping& mapping);
    
    // Controller management
    void addKnownController(const std::string& guid, const std::string& name);
    void removeKnownController(const std::string& guid);
    
    // Event callbacks for GUI integration
    void setControllerConnectedCallback(ControllerConnectedCallback callback);
    void setControllerDisconnectedCallback(ControllerDisconnectedCallback callback);
    void setButtonEventCallback(ButtonEventCallback callback);
    void setStickEventCallback(StickEventCallback callback);
    void setTriggerEventCallback(TriggerEventCallback callback);

private:
    std::unique_ptr<ControllerManager> m_controllerManager;
    std::unique_ptr<MappingManager> m_mappingManager;
    std::unique_ptr<Config> m_config;
    
    // Event callbacks
    ControllerConnectedCallback m_controllerConnectedCallback;
    ControllerDisconnectedCallback m_controllerDisconnectedCallback;
    ButtonEventCallback m_buttonEventCallback;
    StickEventCallback m_stickEventCallback;
    TriggerEventCallback m_triggerEventCallback;
    
    // Internal state tracking
    std::map<std::string, std::string> m_connectedControllers; // guid -> name
    std::map<std::string, std::string> m_previousConnectedControllers; // for change detection
    
    // Time tracking for consistent movement
    std::chrono::steady_clock::time_point m_lastPollTime;
    float m_deltaTime; // Time since last poll in seconds
    
    // Internal methods
    void onControllerConnected(const std::string& guid, const std::string& name);
    void onControllerDisconnected(const std::string& guid);
    void processControllerEvents();
    void updateDeltaTime();
}; 