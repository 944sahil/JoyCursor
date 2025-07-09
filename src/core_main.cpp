#include "core/controller_manager.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <SDL3/SDL.h>

int main() {
    ControllerManager* manager = createControllerManager();
    std::cout << "Controller detection running. Press Enter to exit..." << std::endl;
    std::atomic<bool> running{true};
    std::thread pollThread([&]() {
        while (running) {
            manager->pollEvents();
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
            SDL_Delay(5);
        }
    });
    std::cin.get();
    running = false;
    pollThread.join();
    delete manager;
    return 0;
} 