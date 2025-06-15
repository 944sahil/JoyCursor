#include <SDL2/SDL.h>
#include <iostream>
#include <windows.h>
#include <map>
#include <chrono>

// Constants for timing
const int INITIAL_CLICK_DELAY = 500;  // milliseconds before considering it a hold
const int REPEAT_DELAY = 100;         // milliseconds between repeated key presses

// Function to simulate keyboard input using Windows API
void simulateKeyPress(WORD keyCode, bool isDown) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = isDown ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Structure to track button state and timing
struct ButtonState {
    bool isPressed;
    std::chrono::steady_clock::time_point pressTime;
    bool hasTriggeredInitial;
    bool isHolding;
};

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

    // Map to track D-pad button states
    std::map<int, ButtonState> dpadStates;
    std::map<int, WORD> dpadToKey = {
        {SDL_CONTROLLER_BUTTON_DPAD_UP, VK_UP},
        {SDL_CONTROLLER_BUTTON_DPAD_DOWN, VK_DOWN},
        {SDL_CONTROLLER_BUTTON_DPAD_LEFT, VK_LEFT},
        {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, VK_RIGHT}
    };

    std::cout << "Controller to keyboard mapping active. Press [X] on window or CTRL+C to quit.\n";
    std::cout << "Press SELECT (Back) for ALT key\n";
    std::cout << "Press START (Menu) for TAB key\n";
    std::cout << "D-pad maps to arrow keys (short press for click, hold for repeated presses)\n";

    while (running) {
        auto currentTime = std::chrono::steady_clock::now();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
                    simulateKeyPress(VK_MENU, true);  // ALT key down
                }
                else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    simulateKeyPress(VK_TAB, true);   // TAB key down
                }
                else {
                    auto it = dpadToKey.find(static_cast<int>(event.cbutton.button));
                    if (it != dpadToKey.end()) {
                        // Initialize or update D-pad button state
                        dpadStates[static_cast<int>(event.cbutton.button)] = {
                            true,
                            currentTime,
                            false,
                            false
                        };
                    }
                }
            }
            else if (event.type == SDL_CONTROLLERBUTTONUP) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
                    simulateKeyPress(VK_MENU, false);  // ALT key up
                }
                else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    simulateKeyPress(VK_TAB, false);   // TAB key up
                }
                else {
                    auto it = dpadToKey.find(static_cast<int>(event.cbutton.button));
                    if (it != dpadToKey.end()) {
                        // Release the key if it was being held
                        auto stateIt = dpadStates.find(static_cast<int>(event.cbutton.button));
                        if (stateIt != dpadStates.end() && stateIt->second.isHolding) {
                            simulateKeyPress(it->second, false);
                        }
                        dpadStates.erase(static_cast<int>(event.cbutton.button));
                    }
                }
            }
        }

        // Handle D-pad timing and repeated presses
        for (auto& [button, state] : dpadStates) {
            if (!state.isPressed) continue;

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - state.pressTime).count();

            if (!state.hasTriggeredInitial) {
                // Initial press
                simulateKeyPress(dpadToKey[button], true);
                state.hasTriggeredInitial = true;
            }
            else if (elapsed >= INITIAL_CLICK_DELAY && !state.isHolding) {
                // Start holding after initial delay
                state.isHolding = true;
            }
            else if (state.isHolding && elapsed % REPEAT_DELAY < 10) {
                // Simulate repeated key presses while holding
                simulateKeyPress(dpadToKey[button], false);
                simulateKeyPress(dpadToKey[button], true);
            }
        }

        SDL_Delay(10);  // Small delay to prevent excessive CPU usage
    }

    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
} 