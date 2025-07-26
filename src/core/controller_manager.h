// controller_manager.h
// Interface for managing controllers (platform-independent)

#pragma once
#include <string>
#include <functional>

// Callback types for core integration
using ControllerConnectedCallback = std::function<void(const std::string& guid, const std::string& name)>;
using ControllerDisconnectedCallback = std::function<void(const std::string& guid)>;

class ControllerManager {
public:
    virtual void detectControllers() = 0;
    virtual void pollEvents(float deltaTime = 0.005f) = 0;
    virtual bool hasActiveController() const = 0;
    virtual std::string getActiveControllerName() const = 0;
    
    // Callback setters for core integration
    virtual void setControllerConnectedCallback(ControllerConnectedCallback callback) = 0;
    virtual void setControllerDisconnectedCallback(ControllerDisconnectedCallback callback) = 0;
    
    // Reload mappings from JSON
    virtual void reloadMappings() = 0;
    
    virtual ~ControllerManager() = default;
};

// Factory function to create the implementation
ControllerManager* createControllerManager(); 