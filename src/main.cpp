#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <optional>
#include <vector>

#include <Chip8.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

std::optional<std::uint8_t> getChipEightKey(const SDL_Scancode &key)
{
    //   PC               CHIP8
    // 1 2 3 4           1 2 3 C
    // Q W E R     →     4 5 6 D
    // A S D F           7 8 9 E
    // Z X C V           A 0 B F

    if (key == SDL_SCANCODE_1)
    {
        return 0x1;
    }
    else if (key == SDL_SCANCODE_2)
    {
        return 0x2;
    }
    else if (key == SDL_SCANCODE_3)
    {
        return 0x3;
    }
    else if (key == SDL_SCANCODE_4)
    {
        return 0xC;
    }
    else if (key == SDL_SCANCODE_Q)
    {
        return 0x4;
    }
    else if (key == SDL_SCANCODE_W)
    {
        return 0x5;
    }
    else if (key == SDL_SCANCODE_E)
    {
        return 0x6;
    }
    else if (key == SDL_SCANCODE_R)
    {
        return 0xD;
    }
    else if (key == SDL_SCANCODE_A)
    {
        return 0x7;
    }
    else if (key == SDL_SCANCODE_S)
    {
        return 0x8;
    }
    else if (key == SDL_SCANCODE_D)
    {
        return 0x9;
    }
    else if (key == SDL_SCANCODE_F)
    {
        return 0xE;
    }
    else if (key == SDL_SCANCODE_Z)
    {
        return 0xA;
    }
    else if (key == SDL_SCANCODE_X)
    {
        return 0x0;
    }
    else if (key == SDL_SCANCODE_C)
    {
        return 0xB;
    }
    else if (key == SDL_SCANCODE_V)
    {
        return 0xF;
    }

    return std::nullopt;
}

std::vector<std::int16_t> generateToneBuffer()
{
    std::vector<std::int16_t> sample{};

    for (int i = 0; i < 480; i++)
    {
        auto positionInWave = i % 96;

        if (positionInWave < 48)
        {
            sample.push_back(3000);
        }
        else
        {
            sample.push_back(-3000);
        }
    }

    return sample;
}

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

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        std::cout << "SDL initialization failed: " << SDL_GetError() << "\n";
        return 0;
    }

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    float scale = 15;

    SDL_AudioSpec audioSpec{
        SDL_AUDIO_S16,
        1,
        48000};

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

    SDL_AudioStream *audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, NULL, NULL);

    if (!audioStream)
    {
        std::cout << "Could not open SDL Audio Stream: "
                  << SDL_GetError() << "\n";

        SDL_Quit();
        return 0;
    }

    SDL_ResumeAudioStreamDevice(audioStream);

    std::vector<std::int16_t> sample = generateToneBuffer();
    int toneBufferBytes = sample.size() * sizeof(sample[0]);

    auto startTime = std::chrono::steady_clock::now();

    auto lastTimerTick = startTime;
    auto timerInterval = std::chrono::nanoseconds(1'000'000'000 / 60); // 1 second = 1 bilion nanoseconds

    auto lastCpuTick = startTime;
    auto cpuInterval = std::chrono::nanoseconds(1'000'000'000 / 700);

    auto lastRenderTick = startTime;
    auto renderInterval = std::chrono::nanoseconds(1'000'000'000 / 60);

    bool running = true;
    bool wasSoundActive = false;

    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                auto key_code = getChipEightKey(event.key.scancode);
                if (key_code)
                {
                    chip8.setKeyState(key_code.value(), true);
                }
            }

            if (event.type == SDL_EVENT_KEY_UP)
            {
                auto key_code = getChipEightKey(event.key.scancode);
                if (key_code)
                {
                    chip8.setKeyState(*key_code, false);
                }
            }
        }

        auto currentTime = std::chrono::steady_clock::now();

        while (running && currentTime >= lastCpuTick + cpuInterval)
        {
            lastCpuTick += cpuInterval;
            if (!chip8.cycle())
            {
                std::cout << "Fatal emulation error. Emulator stopped working.\n";
                running = false;
            }
        }

        if (!running)
        {
            break;
        }

        while (currentTime >= lastTimerTick + timerInterval)
        {
            lastTimerTick += timerInterval;
            chip8.tickTimers();
        }

        if (currentTime >= lastRenderTick + renderInterval)
        {
            lastRenderTick = currentTime;

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

        if (chip8.isSoundActive())
        {
            wasSoundActive = true;
            int soundQueueLength = SDL_GetAudioStreamQueued(audioStream);
            if (soundQueueLength < toneBufferBytes)
            {
                SDL_PutAudioStreamData(audioStream, sample.data(), toneBufferBytes);
            }
        }
        else
        {
            if (wasSoundActive)
            {
                SDL_ClearAudioStream(audioStream);
                wasSoundActive = false;
            }
        }

        auto waitTime = std::min({lastCpuTick + cpuInterval, lastTimerTick + timerInterval, lastRenderTick + renderInterval});

        std::this_thread::sleep_until(waitTime);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyAudioStream(audioStream);
    SDL_Quit();

    return 0;
}