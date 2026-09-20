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
        // Load immediate value NN into register VX.
        std::cout << "Loading V" << x << " with " << nn << '\n';
        v_[x] = nn;
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
    }
    else if (family == 7)
    {
        // Add immediate value NN to register VX. Does not effect VF.
        std::cout << "Adding V" << x << " with " << nn << '\n';
        v_[x] += nn;
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
    }
    else if (family == 8 && n == 0)
    {
        // Copy the value in register VY into VX
        std::cout << "Copying V" << y << " into  V" << x << '\n';
        v_[x] = v_[y];
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
        std::cout << "V" << y << ": " << int(v_[y]) << "\n";
    }
    else if (family == 8 && n == 1)
    {
        // Set VX equal to the bitwise OR of the values in VX and VY.
        std::cout << "Bitwise OR of V" << x << "with  V" << y << '\n';
        v_[x] = v_[x] | v_[y];
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
        std::cout << "V" << y << ": " << int(v_[y]) << "\n";
    }
    else if (family == 8 && n == 2)
    {
        // Set VX equal to the bitwise AND of the values in VX and VY.
        std::cout << "Bitwise AND of V" << x << "with  V" << y << '\n';
        v_[x] = v_[x] & v_[y];
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
        std::cout << "V" << y << ": " << int(v_[y]) << "\n";
    }
    else if (family == 8 && n == 3)
    {
        // Set VX equal to the bitwise XOR of the values in VX and VY.
        std::cout << "Bitwise XOR of V" << x << "with  V" << y << '\n';
        v_[x] = v_[x] ^ v_[y];
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
        std::cout << "V" << y << ": " << int(v_[y]) << "\n";
    }
    else if (family == 8 && n == 4)
    {
        // Set VX equal to VX plus VY. In the case of an overflow VF is set to 1. Otherwise 0.
        std::cout << "Adding V" << x << " with V" << y << '\n';
        std::uint8_t aux = v_[x];
        v_[x] += v_[y];
        if (aux > v_[x])
        {
            std::cout << "V" << x << " overflowen, setting Vf to 1.\n";
            v_[0x000f] = 1;
        }
        else
        {
            v_[0x000f] = 0;
        }
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
        std::cout << "Vf: " << int(v_[0x000f]) << "\n";
    }
    else if (family == 8 && n == 5)
    {
        // Set VX equal to VX minus VY. In the case of an underflow VF is set 0. Otherwise 1. (VF = VX > VY)
        std::cout << "Subtracting V" << x << " with V" << y << '\n';
        std::uint8_t aux = v_[x];
        v_[x] -= v_[y];
        if (aux < v_[x])
        {
            std::cout << "V" << x << " underflowen, setting Vf to 0.\n";
            v_[0x000f] = 0;
        }
        else
        {
            v_[0x000f] = 1;
        }
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
        std::cout << "Vf: " << int(v_[0x000f]) << "\n";
    }
    else if (family == 8 && n == 6)
    {
        // Set VX equal to VX bitshifted right 1. VF is set to the least significant bit of VX prior to the shift.
        // Originally this opcode meant set VX equal to VY bitshifted right 1 but emulators and software seem to ignore VY now.
        std::cout << "Setting V" << 0x000f << " equal to the least significant bit of VX.\n";
        v_[0x000f] = v_[x] & 0x01;
        std::cout << "Bitshifting V" << x << " rigth 1 bit.\n";
        v_[x] = v_[x] >> 1;
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
        std::cout << "Vf: " << int(v_[0x000f]) << "\n";
    }
    else if (family == 8 && n == 7)
    {
        // Set VX equal to VY minus VX. VF is set to 1 if VY > VX. Otherwise 0.
        if (v_[y] >= v_[x])
        {
            v_[0x000f] = 1;
        }
        else
        {
            std::cout << "V" << x << " underflow detected, setting Vf to 0.\n";
            v_[0x000f] = 0;
        }
        std::cout << "Subtracting V" << y << " with V" << x << '\n';
        v_[x] = v_[y] - v_[x];
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
        std::cout << "Vf: " << int(v_[0x000f]) << "\n";
    }
    else if (family == 8 && n == 0x000e)
    {
        // Set VX equal to VX bitshifted left 1. VF is set to the most significant bit of VX prior to the shift.
        // Originally this opcode meant set VX equal to VY bitshifted left 1 but emulators and software seem to ignore VY now.
        std::cout << "Setting V" << 0x000f << " equal to the most significant bit of VX.\n";
        v_[0x000f] = (v_[x] & 0x80) >> 7;
        std::cout << "Bitshifting V" << x << " left 1 bit.\n";
        v_[x] = v_[x] << 1;
        std::cout << "V" << x << ": " << int(v_[x]) << "\n";
        std::cout << "Vf: " << int(v_[0x000f]) << "\n";
    }
    else
    {
        std::cout << "opcode not implemented yet.\n";
    }

    pc_ += 2;
}