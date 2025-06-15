#include <SDL2/SDL.h>
#include <iostream>
#include <windows.h>
#include <map>
#include <chrono>

// Constants for scroll sensitivity
const int SCROLL_THRESHOLD = 8000;    // Minimum trigger value to start scrolling
const int SCROLL_DELAY = 16;          // Milliseconds between scroll events (roughly 60fps)
const int MAX_AXIS_VALUE = 32767;     // Maximum value for SDL controller axis

// Structure to track trigger state and timing
struct TriggerState {
    bool isPressed;
    std::chrono::steady_clock::time_point pressTime;
    bool isScrolling;
};

// Function to simulate mouse wheel scroll
void simulateScroll(int amount) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = amount;
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

    // Map to track trigger states
    std::map<int, TriggerState> triggerStates;

    std::cout << "Controller to scroll mapping active. Press [X] on window or CTRL+C to quit.\n";
    std::cout << "Left Trigger: Scroll Up (hold for continuous scroll)\n";
    std::cout << "Right Trigger: Scroll Down (hold for continuous scroll)\n";

    while (running) {
        auto currentTime = std::chrono::steady_clock::now();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_CONTROLLERAXISMOTION) {
                if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || 
                    event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                    
                    if (event.caxis.value > SCROLL_THRESHOLD) {
                        // Initialize or update trigger state
                        triggerStates[static_cast<int>(event.caxis.axis)] = {
                            true,
                            currentTime,
                            false
                        };
                    } else {
                        // Reset trigger state when released
                        triggerStates.erase(static_cast<int>(event.caxis.axis));
                    }
                }
            }
        }

        // Handle continuous scrolling
        for (auto& [axis, state] : triggerStates) {
            if (!state.isPressed) continue;

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - state.pressTime).count();

            if (!state.isScrolling) {
                state.isScrolling = true;
            }

            // Get current trigger value for smooth scrolling
            Sint16 triggerValue = SDL_GameControllerGetAxis(controller, static_cast<SDL_GameControllerAxis>(axis));
            if (triggerValue > SCROLL_THRESHOLD) {
                // Calculate scroll amount based on trigger pressure
                float normalizedValue = static_cast<float>(triggerValue - SCROLL_THRESHOLD) / 
                                     (MAX_AXIS_VALUE - SCROLL_THRESHOLD);
                int scrollAmount = static_cast<int>(normalizedValue * 40); // Max scroll of 40 units per frame
                
                if (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                    simulateScroll(scrollAmount);
                } else {
                    simulateScroll(-scrollAmount);
                }
            }
        }

        SDL_Delay(SCROLL_DELAY);  // Small delay to maintain consistent frame rate
    }

    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
} 