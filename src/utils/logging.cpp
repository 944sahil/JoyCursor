// logging.cpp
// Implementation for logging utilities (to be implemented) 

#include "logging.h"
#include <iostream>

void logInfo(const char* message) {
    std::cout << "[INFO] " << message << std::endl;
}

void logError(const char* message) {
    std::cerr << "[ERROR] " << message << std::endl;
} 