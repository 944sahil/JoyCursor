#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <windows.h>

// Constants for mouse movement
const float RIGHT_STICK_SENSITIVITY = 0.5f;  // Sensitivity for right stick
const float LEFT_STICK_SENSITIVITY = 0.1f;   // Lower sensitivity for left stick
const int DEADZONE = 8000;                  // Ignore small movements below this threshold
const int MAX_AXIS_VALUE = 32767;           // Maximum value for SDL controller axis

// Function to simulate mouse clicks using Windows API
void simulateMouseClick(bool isLeftClick, bool isDown) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = isLeftClick ? 
        (isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP) :
        (isDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP);
    SendInput(1, &input, sizeof(INPUT));
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    // Open the first available controller
    SDL_GameController* controller = nullptr;
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                std::cout << "Opened controller: " << SDL_GameControllerName(controller) << "\n";
                break;
            } else {
                std::cerr << "Could not open controller " << i << ": " << SDL_GetError() << "\n";
            }
        }
    }

    if (!controller) {
        std::cerr << "No compatible game controller found.\n";
        SDL_Quit();
        return 1;
    }

    SDL_Event event;
    bool running = true;

    std::cout << "Controller to mouse mapping active. Press [X] on window or CTRL+C to quit.\n";
    std::cout << "Use the right analog stick for fast mouse movement.\n";
    std::cout << "Use the left analog stick for precise mouse movement.\n";
    std::cout << "Press A for left click and B for right click.\n";

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
                    simulateMouseClick(true, true);  // Left click down
                }
                else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
                    simulateMouseClick(false, true);  // Right click down
                }
            }
            else if (event.type == SDL_CONTROLLERBUTTONUP) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
                    simulateMouseClick(true, false);  // Left click up
                }
                else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
                    simulateMouseClick(false, false);  // Right click up
                }
            }
        }

        // Get stick values
        Sint16 rightX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        Sint16 rightY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
        Sint16 leftX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 leftY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

        // Apply deadzone
        if (std::abs(rightX) < DEADZONE) rightX = 0;
        if (std::abs(rightY) < DEADZONE) rightY = 0;
        if (std::abs(leftX) < DEADZONE) leftX = 0;
        if (std::abs(leftY) < DEADZONE) leftY = 0;

        // Get current mouse position
        int mouseX, mouseY;
        SDL_GetGlobalMouseState(&mouseX, &mouseY);

        // Calculate movement with normalized values
        float normalizedRightX = static_cast<float>(rightX) / MAX_AXIS_VALUE;
        float normalizedRightY = static_cast<float>(rightY) / MAX_AXIS_VALUE;
        float normalizedLeftX = static_cast<float>(leftX) / MAX_AXIS_VALUE;
        float normalizedLeftY = static_cast<float>(leftY) / MAX_AXIS_VALUE;

        // Calculate new position with combined stick movements
        int newX = mouseX + static_cast<int>((normalizedRightX * RIGHT_STICK_SENSITIVITY + 
                                            normalizedLeftX * LEFT_STICK_SENSITIVITY) * 100);
        int newY = mouseY + static_cast<int>((normalizedRightY * RIGHT_STICK_SENSITIVITY + 
                                            normalizedLeftY * LEFT_STICK_SENSITIVITY) * 100);

        // Move the mouse cursor globally
        SDL_WarpMouseGlobal(newX, newY);

        SDL_Delay(10);  // Small delay to prevent excessive CPU usage
    }

    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}