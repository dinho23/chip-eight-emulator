#pragma once
#include <string>
#include <array>
#include <cstdint>

class Chip8
{
public:
    Chip8();

    bool loadRom(const std::string &path);
    bool isSoundActive() const;

    void cycle();
    void tickTimers();

    void setKeyState(std::uint8_t key, bool pressed);

    const std::array<std::array<std::uint8_t, 64>, 32> &getDisplay() const;

private:
    std::array<std::uint8_t, 4096> memory_{};
    std::array<std::uint8_t, 16> v_{};
    std::array<std::uint16_t, 16> stack_{};
    std::array<std::array<std::uint8_t, 64>, 32> display_{}; // 64x32 resolution, means 32 rows, 64 columns
    std::array<bool, 16> keypad_{};

    std::uint16_t pc_ = 0x200;
    std::uint16_t index_ = 0;
    std::uint8_t sp_ = 0;

    std::uint8_t delayTimer_ = 0;
    std::uint8_t soundTimer_ = 0;
    std::uint16_t currentRomLength = 0;
};