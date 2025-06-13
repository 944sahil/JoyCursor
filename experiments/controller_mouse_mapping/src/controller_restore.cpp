#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    std::cout << "Attempting to restore all connected controllers...\n";

    // Try to restore all connected controllers
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            SDL_GameController* controller = SDL_GameControllerOpen(i);
            if (controller) {
                std::cout << "Found controller: " << SDL_GameControllerName(controller) << "\n";
                
                // Re-enable all sensors
                SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_ACCEL, SDL_TRUE);
                SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_GYRO, SDL_TRUE);
                
                // Restore normal player index
                SDL_GameControllerSetPlayerIndex(controller, 0);
                
                // Reset LED to default
                SDL_GameControllerSetLED(controller, 255, 255, 255);
                
                // Disable rumble
                SDL_GameControllerRumble(controller, 0, 0, 0);
                
                std::cout << "Controller functions restored.\n";
                
                SDL_GameControllerClose(controller);
            }
        }
    }

    SDL_Quit();
    std::cout << "Controller restore complete. You can now close this window.\n";
    std::cout << "Press Enter to exit...";
    std::cin.get();
    return 0;
} 