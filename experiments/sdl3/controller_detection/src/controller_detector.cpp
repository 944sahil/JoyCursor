#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gamepad.h>
#include <iostream>
#include <string>
#include <unordered_map>

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    // Get list of connected gamepads
    int count;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
    if (!gamepads || count == 0) {
        std::cerr << "No gamepads found.\n";
        SDL_Quit();
        return 1;
    }

    // Open the first available gamepad
    SDL_Gamepad* gamepad = SDL_OpenGamepad(gamepads[0]);
    if (!gamepad) {
        std::cerr << "Could not open gamepad: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    std::cout << "Opened gamepad: " << SDL_GetGamepadName(gamepad) << "\n";

    SDL_Event event;
    bool running = true;

    std::unordered_map<SDL_GamepadAxis, int> triggerState;
    const int TRIGGER_THRESHOLD = 8000; // Value threshold to start considering press

    std::cout << "Listening for gamepad inputs... Press [X] on window or CTRL+C to quit.\n";

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;

                case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                    std::string state = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ? "pressed" : "released";
                    SDL_GamepadButton button = (SDL_GamepadButton)event.gbutton.button;
                    std::cout << "Button " << SDL_GetGamepadStringForButton(button) << " " << state << "\n";
                    break;
                }

                case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
                    SDL_GamepadAxis axis = (SDL_GamepadAxis)event.gaxis.axis;
                    int value = event.gaxis.value;

                    // Only log analog movement if past a threshold (skip small jitters)
                    if (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
                        if (value > TRIGGER_THRESHOLD) {
                            if (triggerState[axis] <= TRIGGER_THRESHOLD) {
                                std::cout << SDL_GetGamepadStringForAxis(axis) << " triggered: " << value << "\n";
                            }
                            triggerState[axis] = value;
                        } else if (triggerState[axis] > TRIGGER_THRESHOLD) {
                            std::cout << SDL_GetGamepadStringForAxis(axis) << " released\n";
                            triggerState[axis] = 0;
                        }
                    } else {
                        if (abs(value) > 8000) {
                            std::cout << "Axis " << SDL_GetGamepadStringForAxis(axis)
                                      << " moved to " << value << "\n";
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        }

        SDL_Delay(10);
    }

    SDL_CloseGamepad(gamepad);
    SDL_Quit();
    return 0;
} 