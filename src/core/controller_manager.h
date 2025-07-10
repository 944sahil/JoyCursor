// controller_manager.h
// Interface for managing controllers (platform-independent)

#pragma once
#include <string>

class ControllerManager {
public:
    virtual void detectControllers() = 0;
    virtual void pollEvents() = 0;
    virtual bool hasActiveController() const = 0;
    virtual std::string getActiveControllerName() const = 0;
    virtual ~ControllerManager() = default;
};

// Factory function to create the implementation
ControllerManager* createControllerManager(); 