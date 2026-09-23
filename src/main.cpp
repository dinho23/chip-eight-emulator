#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <optional>
#include <vector>

#include <Chip8.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

constexpr int DISPLAY_SCALE = 15;
constexpr int DISPLAY_WIDTH = 64;
constexpr int DISPLAY_HEIGHT = 32;

constexpr int CPU_FREQUENCY = 700;
constexpr int TIMER_FREQUENCY = 60;
constexpr int RENDER_FREQUENCY = 60;

constexpr int AUDIO_SAMPLE_RATE = 48000;
constexpr int AUDIO_TONE_FREQUENCY = 500;
constexpr int AUDIO_BUFFER_MS = 10;
constexpr std::int16_t AUDIO_AMPLITUDE = 3000;

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

    for (int i = 0; i < AUDIO_SAMPLE_RATE / (AUDIO_BUFFER_MS * 10); i++)
    {
        auto positionInWave = i % (AUDIO_SAMPLE_RATE / AUDIO_TONE_FREQUENCY);

        if (positionInWave < AUDIO_SAMPLE_RATE / AUDIO_TONE_FREQUENCY / 2)
        {
            sample.push_back(AUDIO_AMPLITUDE);
        }
        else
        {
            sample.push_back(-AUDIO_AMPLITUDE);
        }
    }

    return sample;
}

bool processEvents(Chip8 &chip8)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            return false;
        }
        else if (event.type == SDL_EVENT_KEY_DOWN)
        {
            auto key_code = getChipEightKey(event.key.scancode);
            if (key_code)
            {
                chip8.setKeyState(key_code.value(), true);
            }
        }
        else if (event.type == SDL_EVENT_KEY_UP)
        {
            auto key_code = getChipEightKey(event.key.scancode);
            if (key_code)
            {
                chip8.setKeyState(*key_code, false);
            }
        }
    }

    return true;
}

bool renderDisplay(SDL_Renderer *renderer, const Chip8 &chip8)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    const auto &display = chip8.getDisplay();
    const auto display_rows = display.size();
    const auto display_cols = display.at(0).size();

    for (int y = 0; y < display_rows; y++)
    {
        for (int x = 0; x < display_cols; x++)
        {
            if (display[y][x] == 1)
            {
                SDL_FRect fillRect = {static_cast<float>(x) * DISPLAY_SCALE, static_cast<float>(y) * DISPLAY_SCALE, static_cast<float>(DISPLAY_SCALE), static_cast<float>(DISPLAY_SCALE)};
                SDL_RenderFillRect(renderer, &fillRect);
            }
        }
    }

    if (!SDL_RenderPresent(renderer))
    {
        return false;
    }

    return true;
}

bool updateAudio(SDL_AudioStream *audioStream, const Chip8 &chip8, const std::vector<std::int16_t> &sample, int toneBufferBytes, bool &wasSoundActive)
{
    if (chip8.isSoundActive())
    {
        wasSoundActive = true;

        int soundQueueLength = SDL_GetAudioStreamQueued(audioStream);
        if (soundQueueLength == -1)
        {
            std::cout << "Failed to queue audio stream: " << SDL_GetError() << "\n";
            return false;
        }

        if (soundQueueLength < toneBufferBytes)
        {
            if (!SDL_PutAudioStreamData(audioStream, sample.data(), toneBufferBytes))
            {
                std::cout << "Failed to queue audio stream: " << SDL_GetError() << "\n";
                return false;
            }
        }
    }
    else
    {
        if (wasSoundActive)
        {
            wasSoundActive = false;
            if (!SDL_ClearAudioStream(audioStream))
            {
                std::cout << "Failed to clear audio stream: " << SDL_GetError() << "\n";
                return false;
            }
        }
    }

    return true;
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

    SDL_AudioSpec audioSpec{
        SDL_AUDIO_S16,
        1,
        AUDIO_SAMPLE_RATE};

    if (!SDL_CreateWindowAndRenderer(
            "CHIP-8 Emulator",
            DISPLAY_WIDTH * DISPLAY_SCALE,
            DISPLAY_HEIGHT * DISPLAY_SCALE,
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

    if (!SDL_ResumeAudioStreamDevice(audioStream))
    {
        std::cout << "Failed to resume audio stream: " << SDL_GetError() << "\n";
    }

    std::vector<std::int16_t> sample = generateToneBuffer();
    int toneBufferBytes = sample.size() * sizeof(sample[0]);

    auto startTime = std::chrono::steady_clock::now();

    auto lastTimerTick = startTime;
    auto timerInterval = std::chrono::nanoseconds(1'000'000'000 / TIMER_FREQUENCY); // 1 second = 1 bilion nanoseconds

    auto lastCpuTick = startTime;
    auto cpuInterval = std::chrono::nanoseconds(1'000'000'000 / CPU_FREQUENCY);

    auto lastRenderTick = startTime;
    auto renderInterval = std::chrono::nanoseconds(1'000'000'000 / RENDER_FREQUENCY);

    bool running = true;
    bool wasSoundActive = false;

    while (running)
    {
        if (!processEvents(chip8))
        {
            running = false;
        }

        if (!running)
        {
            break;
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

            running = renderDisplay(renderer, chip8);
        }

        if (!running)
        {
            break;
        }

        if (!updateAudio(audioStream, chip8, sample, toneBufferBytes, wasSoundActive))
        {
            running = false;
        }

        if (!running)
        {
            break;
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