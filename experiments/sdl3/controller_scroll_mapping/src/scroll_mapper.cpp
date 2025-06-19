#include <SDL3/SDL.h>
#include <windows.h>
#include <iostream>
#include <chrono>
#include <cmath>

// Constants
const int TRIGGER_THRESHOLD = 8000;
const int MAX_AXIS = 32767;
const int SCROLL_INTERVAL_MS = 10;
const float BASE_SCROLL_PER_FRAME = 2.0f;
const float MAX_SCROLL_PER_FRAME = 40.0f;
const float MAX_ACCEL_TIME = 2000.0f; // in milliseconds

// Windows scroll simulation
void simulateMouseScroll(int amount) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = amount;
    SendInput(1, &input, sizeof(INPUT));
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_EVENTS) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Gamepad* gamepad = nullptr;

    // Wait for a gamepad to be connected
    while (!gamepad) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                gamepad = SDL_OpenGamepad(event.gdevice.which);
                if (gamepad) {
                    std::cout << "Gamepad connected.\n";
                }
            } else if (event.type == SDL_EVENT_QUIT) {
                SDL_Quit();
                return 0;
            }
        }
        SDL_Delay(100);
    }

    std::cout << "Trigger scroll ready: Hold Left Trigger to scroll up, Right Trigger to scroll down.\n";

    SDL_Event event;
    bool running = true;

    std::chrono::steady_clock::time_point lastScrollTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point leftTriggerStart{}, rightTriggerStart{};
    bool leftScrolling = false, rightScrolling = false;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_GAMEPAD_REMOVED) {
                running = false;
            }
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastScrollTime).count();

        if (elapsed >= SCROLL_INTERVAL_MS) {
            lastScrollTime = now;

            Sint16 lt = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
            Sint16 rt = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

            int scrollAmount = 0;

            // Left trigger scroll up
            if (lt > TRIGGER_THRESHOLD) {
                if (!leftScrolling) {
                    leftTriggerStart = now;
                    leftScrolling = true;
                }

                float normPressure = (lt - TRIGGER_THRESHOLD) / static_cast<float>(MAX_AXIS - TRIGGER_THRESHOLD);
                float heldTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - leftTriggerStart).count();
                float accel = std::min(1.0f, heldTime / MAX_ACCEL_TIME);
                float factor = accel * accel; // quadratic ramp-up

                scrollAmount = static_cast<int>(BASE_SCROLL_PER_FRAME * normPressure * factor * MAX_SCROLL_PER_FRAME);
            } else {
                leftScrolling = false;
            }

            // Right trigger scroll down
            if (rt > TRIGGER_THRESHOLD) {
                if (!rightScrolling) {
                    rightTriggerStart = now;
                    rightScrolling = true;
                }

                float normPressure = (rt - TRIGGER_THRESHOLD) / static_cast<float>(MAX_AXIS - TRIGGER_THRESHOLD);
                float heldTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - rightTriggerStart).count();
                float accel = std::min(1.0f, heldTime / MAX_ACCEL_TIME);
                float factor = accel * accel;

                scrollAmount = -static_cast<int>(BASE_SCROLL_PER_FRAME * normPressure * factor * MAX_SCROLL_PER_FRAME);
            } else {
                rightScrolling = false;
            }

            // Apply scroll
            if (scrollAmount != 0) {
                simulateMouseScroll(scrollAmount);
            }
        }

        SDL_Delay(1);
    }

    SDL_CloseGamepad(gamepad);
    SDL_Quit();
    return 0;
}
