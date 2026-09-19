#include <Chip8.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <bitset>

Chip8::Chip8()
{
}

bool Chip8::loadRom(const std::string &path)
{
    std::cout << "Loading ROM: " << path << "\n";
    std::ifstream input_stream;

    input_stream.open(path, std::ifstream::binary);

    if (input_stream)
    {
        input_stream.seekg(0, input_stream.end);
        auto length = input_stream.tellg();

        // Check if length is > 3584 bytes and reject the ROM in this case
        if (length > 3584)
        {
            std::cout << "ROM file is too large: " << length << " bytes read.";
            return false;
        }
        else if (length < 0)
        {
            std::cout << "Unexpected error while reading the stream, ROM not loaded.";
            return false;
        }

        input_stream.seekg(0, input_stream.beg);

        std::vector<char> buffer{};
        buffer.resize(length);

        std::cout << "Reading " << length << " characters...\n";
        input_stream.read(buffer.data(), length);

        if (input_stream)
        {
            std::cout << "All characters read successfully!\n";
            this->currentRomLength = length;

            size_t j = 0;
            for (uint16_t i = this->pc_; i < pc_ + length; i++)
            {
                this->memory_.at(i) = buffer.at(j);
                j++;
            }
            std::cout << "Successfully loaded ROM into memory.\n";

            return true;
        }
        else
        {
            std::cout << "Error, only " << input_stream.gcount() << " could be read.\n";
        }
    }
    else
    {
        std::cout << "Error while opening the file: " << path << "\n";
    }

    return false;
}

void Chip8::cycle()
{
    std::uint16_t opCode = 0;

    std::cout << std::hex;

    opCode = (std::uint16_t(memory_.at(pc_)) << 8) | memory_.at(pc_ + 1);
    std::cout << "PC " << pc_ << " -> opcode " << opCode << "\n";

    std::uint16_t family = (opCode & 0xf000) >> 12;
    std::uint16_t x = (opCode & 0x0f00) >> 8;
    std::uint16_t y = (opCode & 0x00f0) >> 4;
    std::uint16_t n = opCode & 0x000f;
    std::uint16_t nnn = opCode & 0x0fff;
    std::uint16_t nn = opCode & 0x00ff;

    std::cout << "FAMILY: " << family << "\n";
    std::cout << "X: " << x << "\n";
    std::cout << "Y: " << y << "\n";
    std::cout << "N: " << n << "\n";
    std::cout << "NNN: " << nnn << "\n";
    std::cout << "NN: " << nn << "\n";

    if (family == 6)
    {
        std::cout << "Loading V" << x << " with " << nn << '\n';
        v_[x] = nn;
    }
    else
    {
        std::cout << "opcode not implemented yet.\n";
    }

    pc_ += 2;
}