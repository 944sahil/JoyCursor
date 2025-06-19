#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
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

    SDL_Event event;
    bool running = true;

    // Map to track D-pad button states
    std::map<int, ButtonState> dpadStates;
    std::map<int, WORD> dpadToKey = {
        {SDL_GAMEPAD_BUTTON_DPAD_UP, VK_UP},
        {SDL_GAMEPAD_BUTTON_DPAD_DOWN, VK_DOWN},
        {SDL_GAMEPAD_BUTTON_DPAD_LEFT, VK_LEFT},
        {SDL_GAMEPAD_BUTTON_DPAD_RIGHT, VK_RIGHT}
    };

    std::cout << "Controller to keyboard mapping active. Press BACK button to quit.\n";
    std::cout << "Press BACK for ALT key, START for TAB key. D-pad maps to arrow keys.\n";
    std::cout << "X = Enter, B = Escape.\n";

    while (running) {
        auto currentTime = std::chrono::steady_clock::now();

        SDL_UpdateGamepads();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                if (event.gbutton.button == SDL_GAMEPAD_BUTTON_BACK) {
                    simulateKeyPress(VK_MENU, true);  // ALT key down
                }
                else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_START) {
                    simulateKeyPress(VK_TAB, true);   // TAB key down
                }
                else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_WEST) { // X button
                    simulateKeyPress(VK_RETURN, true); // Enter key down
                }
                else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST) { // B button
                    simulateKeyPress(VK_ESCAPE, true); // Escape key down
                }
                else {
                    auto it = dpadToKey.find(static_cast<int>(event.gbutton.button));
                    if (it != dpadToKey.end()) {
                        dpadStates[static_cast<int>(event.gbutton.button)] = {
                            true,
                            currentTime,
                            false,
                            false
                        };
                    }
                }
            }
            else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
                if (event.gbutton.button == SDL_GAMEPAD_BUTTON_BACK) {
                    simulateKeyPress(VK_MENU, false);  // ALT key up
                }
                else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_START) {
                    simulateKeyPress(VK_TAB, false);   // TAB key up
                }
                else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_WEST) { // X button
                    simulateKeyPress(VK_RETURN, false); // Enter key up
                }
                else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST) { // B button
                    simulateKeyPress(VK_ESCAPE, false); // Escape key up
                }
                else {
                    auto it = dpadToKey.find(static_cast<int>(event.gbutton.button));
                    if (it != dpadToKey.end()) {
                        auto stateIt = dpadStates.find(static_cast<int>(event.gbutton.button));
                        if (stateIt != dpadStates.end() && stateIt->second.isHolding) {
                            simulateKeyPress(it->second, false);
                        }
                        dpadStates.erase(static_cast<int>(event.gbutton.button));
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
                simulateKeyPress(dpadToKey[button], true);
                state.hasTriggeredInitial = true;
            }
            else if (elapsed >= INITIAL_CLICK_DELAY && !state.isHolding) {
                state.isHolding = true;
            }
            else if (state.isHolding && elapsed % REPEAT_DELAY < 10) {
                simulateKeyPress(dpadToKey[button], false);
                simulateKeyPress(dpadToKey[button], true);
            }
        }

        SDL_Delay(10);
    }

    SDL_CloseGamepad(gamepad);
    SDL_Quit();
    return 0;
} 