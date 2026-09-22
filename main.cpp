#include <iostream>
#include <chrono>
#include <Chip8.h>

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

    auto startTime = std::chrono::steady_clock::now();

    auto lastTimerTick = startTime;
    auto timerInterval = std::chrono::nanoseconds(1'000'000'000 / 60); // 1 second = 1 bilion nanoseconds

    auto lastCpuTick = startTime;
    auto cpuInterval = std::chrono::nanoseconds(1'000'000'000 / 700);

    while (true)
    {
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
    }

    return 0;
}