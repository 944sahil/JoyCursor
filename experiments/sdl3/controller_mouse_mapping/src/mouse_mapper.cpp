#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <cmath>
#include <windows.h>

// Constants for mouse movement smoothing
const float LEFT_STICK_SENSITIVITY = 0.05f;  // Lower sensitivity for precise movement
const float RIGHT_STICK_SENSITIVITY = 0.3f;  // Higher sensitivity for fast movement
const float SMOOTHING = 0.2f;          // Damping factor (0 - 1)
const int DEADZONE = 8000;
const int MAX_AXIS_VALUE = 32767;

// Function to simulate mouse clicks using Windows API
void simulateMouseClick(bool isLeftClick, bool isDown) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = isLeftClick ? 
        (isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP) :
        (isDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP);
    SendInput(1, &input, sizeof(INPUT));
}

float applyDeadzone(Sint16 value, int deadzone) {
    return (std::abs(value) < deadzone) ? 0.0f : static_cast<float>(value) / MAX_AXIS_VALUE;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) < 0) {
        std::cerr << "SDL Init Failed: " << SDL_GetError() << "\n";
        return 1;
    }

    if (!SDL_HasGamepad()) {
        std::cerr << "No gamepad detected.\n";
        SDL_Quit();
        return 1;
    }

    int num_gamepads = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&num_gamepads);
    if (!gamepads || num_gamepads == 0) {
        std::cerr << "Failed to list gamepads.\n";
        SDL_Quit();
        return 1;
    }

    SDL_Gamepad* gamepad = SDL_OpenGamepad(gamepads[0]);
    SDL_free(gamepads);
    if (!gamepad) {
        std::cerr << "Failed to open gamepad: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    std::cout << "Using gamepad: " << SDL_GetGamepadName(gamepad) << "\n";

    bool running = true;
    SDL_Event event;

    float velocityX = 0.0f;
    float velocityY = 0.0f;

    std::cout << "Move mouse with left stick (precise) or right stick (fast). Press BACK button to quit.\n";
    std::cout << "Press A for left click and Right Shoulder for right click.\n";

    while (running) {
        SDL_UpdateGamepads();  // Important for polling!

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN &&
                event.gbutton.button == SDL_GAMEPAD_BUTTON_BACK) {
                running = false;
            }
            else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {  // A button
                    simulateMouseClick(true, true);  // Left click down
                }
                else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
                    simulateMouseClick(false, true);  // Right click down
                }
            }
            else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
                if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {  // A button
                    simulateMouseClick(true, false);  // Left click up
                }
                else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
                    simulateMouseClick(false, false);  // Right click up
                }
            }
        }

        // Get both stick values
        float leftX = applyDeadzone(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX), DEADZONE);
        float leftY = applyDeadzone(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY), DEADZONE);
        float rightX = applyDeadzone(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX), DEADZONE);
        float rightY = applyDeadzone(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY), DEADZONE);

        // Combine stick movements with different sensitivities
        float combinedX = leftX * LEFT_STICK_SENSITIVITY + rightX * RIGHT_STICK_SENSITIVITY;
        float combinedY = leftY * LEFT_STICK_SENSITIVITY + rightY * RIGHT_STICK_SENSITIVITY;

        // Apply smoothing (lerp style)
        velocityX = velocityX * (1.0f - SMOOTHING) + combinedX * SMOOTHING * 100;
        velocityY = velocityY * (1.0f - SMOOTHING) + combinedY * SMOOTHING * 100;

        // Get current mouse position
        float mouseX, mouseY;
        SDL_GetGlobalMouseState(&mouseX, &mouseY);

        // Move the cursor
        SDL_WarpMouseGlobal(mouseX + velocityX, mouseY + velocityY);

        SDL_Delay(5);  // Lower delay for smoother motion
    }

    SDL_CloseGamepad(gamepad);
    SDL_Quit();
    return 0;
}
