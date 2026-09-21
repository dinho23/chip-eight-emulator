#include <Chip8.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <random>

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

    std::cout << "OPCODE: " << opCode << "\n";
    std::cout << "FAMILY: " << family << "\n";
    std::cout << "X: " << x << "\n";
    std::cout << "Y: " << y << "\n";
    std::cout << "N: " << n << "\n";
    std::cout << "NNN: " << nnn << "\n";
    std::cout << "NN: " << nn << "\n";

    bool pc_increment_handled = false;

    if (opCode == 0x00EE)
    {
        // Return from subroutine. Set the PC to the address at the top of the stack and subtract 1 from the SP.

        // Check whether there is a return address on the stack.
        if (sp_ == 0)
        {
            std::cout << "Invalid return instruction given, stack is empty.\n";
            return;
        }

        std::cout << "Returing from subroutine";
        sp_ -= 1;
        pc_ = stack_.at(sp_);
        stack_.at(sp_) = 0;
        pc_increment_handled = true;
    }
    else if (family == 1)
    {
        // Set PC to NNN.
        pc_ = nnn;
        pc_increment_handled = true;
    }
    else if (family == 2)
    {
        // Call subroutine a NNN.
        // Increment the SP and put the current PC value on the top of the stack.
        // Then set the PC to NNN. Generally there is a limit of 16 successive calls.

        // Check whether the stack has space for another return address.
        if (sp_ == stack_.size())
        {
            std::cout << "Invalid call instruction given, stack is full.\n";
            return;
        }

        stack_.at(sp_) = pc_ + 2;
        sp_ += 1;
        pc_ = nnn;
        pc_increment_handled = true;
    }
    else if (family == 3)
    {
        // Skip the next instruction if register VX is equal to NN.
        if (v_[x] == nn)
        {
            pc_ += 2;
        }
    }
    else if (family == 4)
    {
        // Skip the next instruction if register VX is not equal to NN.
        if (v_[x] != nn)
        {
            pc_ += 2;
        }
    }
    else if (family == 5 && n == 0)
    {
        // Skip the next instruction if register VX equals VY.
        if (v_[x] == v_[y])
        {
            pc_ += 2;
        }
    }
    else if (family == 6)
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
    else if (family == 9)
    {
        if (n == 0)
        {
            if (v_[x] != v_[y])
            {
                pc_ += 2;
            }
        }
    }
    else if (family == 0x000a)
    {
        // Set I equal to NNN.
        std::cout << "Setting Index: " << nnn << "\n";
        index_ = nnn;
    }
    else if (family == 0x000b)
    {
        // Set the PC to NNN plus the value in V0.
        std::cout << "Setting PC: " << nnn << " + " << int(v_.at(0)) << "\n";
        pc_ = nnn + v_.at(0);
        pc_increment_handled = true;
    }
    else if (family == 0x000c)
    {
        // Set VX equal to a random number ranging from 0 to 255 which is logically anded with NN.
        std::random_device rd;
        std::uniform_int_distribution<int> distribution(0, 255);
        std::uint8_t random_number = distribution(rd);

        std::cout << std::dec;
        std::cout << "Setting V" << x << " to randomly generated number: " << int(random_number) << " ANDed with " << nn << "\n";
        std::cout << std::hex;
        v_.at(x) = random_number & nn;
    }
    else if (family == 0x000f && nn == 0x0055)
    {
        // Store registers V0 through VX in memory starting at location I.
        // I does not change.
        std::cout << "Copying from registers into memory. Start index: " << index_ << " End index: " << index_ + x << "\n";

        for (std::uint16_t i = 0; i <= x; i++)
        {
            memory_.at(i + index_) = v_.at(i);
        }
    }
    else if (family == 0x000f && nn == 0x0065)
    {
        // Copy values from memory location I through I + X into registers V0 through VX.
        // I does not change.
        std::cout << "Copying from memory into registers. Start index: " << index_ << " End index: " << index_ + x << "\n";

        for (std::uint16_t i = 0; i <= x; i++)
        {
            v_.at(i) = memory_.at(i + index_);
        }
    }
    else
    {
        std::cout << "opcode not implemented yet.\n";
    }

    if (!pc_increment_handled)
    {
        pc_ += 2;
    }

    std::cout << "PC: " << pc_ << "\n";
    std::cout << "SP: " << int(sp_) << "\n";
    std::cout << "INDEX: " << int(index_) << "\n";
    std::cout << "STACK: ";
    for (const auto &item : stack_)
    {
        std::cout << int(item) << " ";
    }
    std::cout << "\n";
    std::cout << "REGISTERS: ";
    for (const auto &item : v_)
    {
        std::cout << int(item) << " ";
    }
    std::cout << "\n";
    std::cout << "MEMORY: ";
    for (size_t i = 0; i < 4096; i++)
    {
        if (memory_.at(i) != 0)
        {
            std::cout << "M[" << i << "]: " << int(memory_.at(i)) << "    ";
        }
    }
    std::cout << "\n";
}