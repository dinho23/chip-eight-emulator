#include <Chip8.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <random>

Chip8::Chip8()
{
    // Load digits sprite into memory from address 0x0000 to 0x004F.
    // A digit occupies 8x5 grid of pixels, so 5 locations in memory are ocuppied by one digit.

    // Load digit 0.
    memory_[0x0000] = 0xF0;
    memory_[0x0001] = memory_[0x0002] = memory_[0x0003] = 0x90;
    memory_[0x0004] = 0xF0;

    // Load digit 1.
    memory_[0x0005] = memory_[0x0007] = memory_[0x0008] = 0x20;
    memory_[0x0006] = 0x60;
    memory_[0x0009] = 0x70;

    // Load digit 2.
    memory_[0x000A] = memory_[0x000C] = memory_[0x000E] = 0xF0;
    memory_[0x000B] = 0x10;
    memory_[0x000D] = 0x80;

    // Load digit 3.
    memory_[0x000F] = memory_[0x0011] = memory_[0x0013] = 0xF0;
    memory_[0x0010] = memory_[0x0012] = 0x10;

    // Load digit 4.
    memory_[0x0014] = memory_[0x0015] = 0x90;
    memory_[0x0016] = 0xF0;
    memory_[0x0017] = memory_[0x0018] = 0x10;

    // Load digit 5.
    memory_[0x0019] = memory_[0x001B] = memory_[0x001D] = 0xF0;
    memory_[0x001A] = 0x80;
    memory_[0x001C] = 0x10;

    // Load digit 6.
    memory_[0x001E] = memory_[0x0020] = memory_[0x0022] = 0xF0;
    memory_[0x001F] = 0x80;
    memory_[0x0021] = 0x90;

    // Load digit 7.
    memory_[0x0023] = 0xF0;
    memory_[0x0024] = 0x10;
    memory_[0x0025] = 0x20;
    memory_[0x0026] = memory_[0x0027] = 0x40;

    // Load digit 8.
    memory_[0x0028] = memory_[0x002A] = memory_[0x002C] = 0xF0;
    memory_[0x0029] = memory_[0x002B] = 0x90;

    // Load digit 9.
    memory_[0x002D] = memory_[0x002F] = memory_[0x0031] = 0xF0;
    memory_[0x002E] = 0x90;
    memory_[0x0030] = 0x10;

    // Load digit A.
    memory_[0x0032] = memory_[0x0034] = 0xF0;
    memory_[0x0033] = memory_[0x0035] = memory_[0x0036] = 0x90;

    // Load digit B.
    memory_[0x0037] = memory_[0x0039] = memory_[0x003B] = 0xE0;
    memory_[0x0038] = memory_[0x003A] = 0x90;

    // Load digit C.
    memory_[0x003C] = memory_[0x0040] = 0xF0;
    memory_[0x003D] = memory_[0x003E] = memory_[0x003F] = 0x80;

    // Load digit D.
    memory_[0x0041] = memory_[0x0045] = 0xE0;
    memory_[0x0042] = memory_[0x0043] = memory_[0x0044] = 0x90;

    // Load digit E.
    memory_[0x0046] = memory_[0x0048] = memory_[0x004A] = 0xF0;
    memory_[0x0047] = memory_[0x0049] = 0x80;

    // Load digit F.
    memory_[0x004B] = memory_[0x004D] = 0xF0;
    memory_[0x004C] = memory_[0x004E] = memory_[0x004F] = 0x80;
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
    else if (opCode == 0x00E0)
    {
        std::cout << "Clearing display.\n";
        for (auto &item : display_)
        {
            item.fill(0);
        }
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
    else if (family == 0x000d)
    {
        // Display N-byte sprite starting at memory location I at (VX, VY).
        // Each set bit of XOR-ed with what's already drawn.
        // VF is set to 1 if a collision occurs. 0 otherwise.
        std::cout << "Drawing " << n << "-byte sprite at (" << int(v_.at(x)) << "," << int(v_.at(y)) << ")\n";

        v_.at(0x000f) = 0;

        for (std::uint8_t i = 0; i < n; i++)
        {
            std::uint8_t sprite_byte = memory_[index_ + i];

            for (std::uint8_t j = 0; j < 8; j++)
            {
                std::uint8_t sprite_pixel = (sprite_byte >> (7 - j)) & 1;

                std::uint8_t screen_y = (v_[y] + i) % 32;
                std::uint8_t screen_x = (v_[x] + j) % 64;

                auto old_pixel = display_[screen_y][screen_x];
                display_.at(screen_y).at(screen_x) = old_pixel ^ sprite_pixel;

                if (old_pixel == 1 && sprite_pixel == 1)
                {
                    v_.at(0x000f) = 1;
                }
            }
        }
    }
    else if (family == 0x000e && nn == 0x009e)
    {
        // Skip the following instruction if the key represented by the value in VX is pressed.
        if (keypad_.at(v_.at(x)))
        {
            std::cout << "Keypad " << v_.at(x) << " pressed. Skipping next instruction.\n";
            pc_ += 2;
        }
    }
    else if (family == 0x000e && nn == 0x00a1)
    {
        // Skip the following instruction if the key represented by the value in VX is not pressed.
        if (!keypad_.at(v_.at(x)))
        {
            std::cout << "Keypad " << v_.at(x) << " not pressed. Skipping next instruction.\n";
            pc_ += 2;
        }
    }
    else if (family == 0x000f && nn == 0x0007)
    {
        // Set VX equal to the delay timer.
        std::cout << "Setting V" << x << " equal to delay timer: " << delayTimer_ << "\n";
        v_.at(x) = delayTimer_;
    }
    else if (family == 0x000f && nn == 0x000a)
    {
        pc_increment_handled = true;
        // Wait for a key press and store the value of the key into VX.
        for (std::size_t i = 0; i < keypad_.size(); i++)
        {
            if (keypad_.at(i))
            {
                v_.at(x) = i;
                pc_increment_handled = false;
                break;
            }
        }
    }
    else if (family == 0x000f && nn == 0x0015)
    {
        // Set the delay timer DT to VX.
        std::cout << "Setting delay timer to V" << x << ": " << int(v_.at(x)) << "\n";
        delayTimer_ = v_.at(x);
    }
    else if (family == 0x000f && nn == 0x0018)
    {
        // Set the sound timer ST to VX.
        std::cout << "Setting sound timer to V" << x << ": " << int(v_.at(x)) << "\n";
        soundTimer_ = v_.at(x);
    }
    else if (family == 0x000f && nn == 0x001e)
    {
        // Add VX to I.
        // VF is set to 1 if I > 0x0FFF. Otherwise set to 0.
        std::cout << "Adding V" << x << ": " << int(v_.at(x)) << "to Index.\n";

        index_ += v_.at(x);

        if (index_ > 0x0fff)
        {
            v_.at(0x000f) = 1;
        }
        else
        {
            v_.at(0x000f) = 0;
        }
    }
    else if (family == 0x000f && nn == 0x0029)
    {
        // Set I = location of sprite for digit Vx.
        // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
        // Memory location for the digits starts at 0x00 through 0x4F, and they are sorted ascending.
        // Sprites are 8x5 meaning each digit starts at an address multiple of 5.
        std::uint8_t digit = v_.at(x) & 0x0f;

        index_ = digit * 5;
    }
    else if (family == 0x000f && nn == 0x0033)
    {
        // Convert the value stored in VX to BCD and store the 3 digits at memory location I through I+2.
        // I does not change.
        std::cout << "Converting V" << x << ": " << int(v_.at(x)) << " to BCD.\n";

        memory_[index_] = v_.at(x) / 100;
        memory_[index_ + 1] = (v_.at(x) / 10) % 10;
        memory_[index_ + 2] = v_.at(x) % 10;
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
    // std::cout << "DISPLAY:\n";
    // for (size_t i = 0; i < 32; i++)
    // {
    //     for (size_t j = 0; j < 64; j++)
    //     {
    //         if (display_.at(i).at(j) == 0)
    //         {
    //             std::cout << ". ";
    //         }
    //         else
    //         {
    //             std::cout << "# ";
    //         }
    //     }
    //     std::cout << "\n";
    // }
    // std::cout << "\n";
}