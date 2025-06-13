#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <unordered_map>

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

    std::unordered_map<SDL_GameControllerAxis, int> triggerState;
    const int TRIGGER_THRESHOLD = 8000; // Value threshold to start considering press

    std::cout << "Listening for controller inputs... Press [X] on window or CTRL+C to quit.\n";

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_CONTROLLERBUTTONDOWN:
                case SDL_CONTROLLERBUTTONUP: {
                    std::string state = event.type == SDL_CONTROLLERBUTTONDOWN ? "pressed" : "released";
                    SDL_GameControllerButton button = (SDL_GameControllerButton)event.cbutton.button;
                    std::cout << "Button " << SDL_GameControllerGetStringForButton(button) << " " << state << "\n";
                    break;
                }

                case SDL_CONTROLLERAXISMOTION: {
                    SDL_GameControllerAxis axis = (SDL_GameControllerAxis)event.caxis.axis;
                    int value = event.caxis.value;

                    // Only log analog movement if past a threshold (skip small jitters)
                    if (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                        if (value > TRIGGER_THRESHOLD) {
                            if (triggerState[axis] <= TRIGGER_THRESHOLD) {
                                std::cout << SDL_GameControllerGetStringForAxis(axis) << " triggered: " << value << "\n";
                            }
                            triggerState[axis] = value;
                        } else if (triggerState[axis] > TRIGGER_THRESHOLD) {
                            std::cout << SDL_GameControllerGetStringForAxis(axis) << " released\n";
                            triggerState[axis] = 0;
                        }
                    } else {
                        if (abs(value) > 8000) {
                            std::cout << "Axis " << SDL_GameControllerGetStringForAxis(axis)
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

    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
