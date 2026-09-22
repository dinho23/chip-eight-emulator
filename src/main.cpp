#include <iostream>
#include <chrono>

#include <Chip8.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        std::cout << "Too many arguments were given.";
        return 0;
    }
    else if (argc < 2)
    {
        std::cout << "Please give the ROM name";
        return 0;
    }

    Chip8 chip8;

    if (!chip8.loadRom(argv[1]))
    {
        return 0;
    }

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cout << "SDL initialization failed: " << SDL_GetError() << "\n";
        return 0;
    }

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    float scale = 15;

    if (!SDL_CreateWindowAndRenderer(
            "CHIP-8 Emulator",
            64 * scale,
            32 * scale,
            0,
            &window,
            &renderer))
    {
        std::cout << "Could not create SDL window: "
                  << SDL_GetError() << "\n";

        SDL_Quit();
        return 0;
    }

    auto startTime = std::chrono::steady_clock::now();

    auto lastTimerTick = startTime;
    auto timerInterval = std::chrono::nanoseconds(1'000'000'000 / 60); // 1 second = 1 bilion nanoseconds

    auto lastCpuTick = startTime;
    auto cpuInterval = std::chrono::nanoseconds(1'000'000'000 / 700);

    bool running = true;

    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }

        auto currentTime = std::chrono::steady_clock::now();

        while (currentTime >= lastCpuTick + cpuInterval)
        {
            lastCpuTick += cpuInterval;
            chip8.cycle();
        }

        while (currentTime >= lastTimerTick + timerInterval)
        {
            lastTimerTick += timerInterval;
            chip8.tickTimers();
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        const auto &display = chip8.getDisplay();
        for (int y = 0; y < 32; y++)
        {
            for (int x = 0; x < 64; x++)
            {
                if (display[y][x] == 1)
                {
                    SDL_FRect fillRect = {static_cast<float>(x) * scale, static_cast<float>(y) * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &fillRect);
                }
            }
        }
        if (!SDL_RenderPresent(renderer))
        {
            running = false;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}