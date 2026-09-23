#include <Chip8.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <random>

#ifdef DEBUG
#define LOG_DEBUG(x) std::cout << "[DEBUG] " << x
#else
#define LOG_DEBUG(x) ((void)0)
#endif

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
    memory_[0xF] = memory_[0x0011] = memory_[0x0013] = 0xF0;
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
    LOG_DEBUG("Loading ROM: " << path << "\n");
    std::ifstream input_stream;

    input_stream.open(path, std::ifstream::binary);

    if (input_stream)
    {
        input_stream.seekg(0, input_stream.end);
        auto length = input_stream.tellg();

        // Check if length is > 3584 bytes and reject the ROM in this case
        if (length > 3584)
        {
            LOG_DEBUG("ROM file is too large: " << length << " bytes read.\n");
            return false;
        }
        else if (length < 0)
        {
            LOG_DEBUG("Unexpected error while reading the stream, ROM not loaded.\n");
            return false;
        }

        input_stream.seekg(0, input_stream.beg);

        std::vector<char> buffer{};
        buffer.resize(length);

        LOG_DEBUG("Reading " << length << " characters...\n");
        input_stream.read(buffer.data(), length);

        if (input_stream)
        {
            LOG_DEBUG("All characters read successfully!\n");

            size_t j = 0;

            if (pc_ + length > memory_.size())
            {
                LOG_DEBUG("Buffer exceeds available memory. ROM too large.\n");
                return false;
            }
            else
            {
                for (uint16_t i = pc_; i < pc_ + length; i++)
                {
                    memory_.at(i) = buffer.at(j);
                    j++;
                }
            }

            LOG_DEBUG("Successfully loaded ROM into memory.\n");

            return true;
        }
        else
        {
            LOG_DEBUG("Error, only " << input_stream.gcount() << " could be read.\n");
        }
    }
    else
    {
        LOG_DEBUG("Error while opening the file: " << path << "\n");
    }

    return false;
}

bool Chip8::isSoundActive() const
{
    return soundTimer_ > 0;
}

bool Chip8::cycle()
{
    if (static_cast<std::size_t>(pc_) + 1 >= memory_.size())
    {
        LOG_DEBUG("PC out of bounds: " << pc_ << "\n");
        return false;
    }

    std::uint16_t opCode = 0;

    std::cout << std::hex;

    opCode = (std::uint16_t(memory_.at(pc_)) << 8) | memory_.at(pc_ + 1);
    LOG_DEBUG("PC " << pc_ << " -> opcode " << opCode << "\n");

    std::uint16_t family = (opCode & 0xf000) >> 12;
    std::uint16_t x = (opCode & 0x0f00) >> 8;
    std::uint16_t y = (opCode & 0x00f0) >> 4;
    std::uint16_t n = opCode & 0xF;
    std::uint16_t nnn = opCode & 0x0fff;
    std::uint16_t nn = opCode & 0x00ff;

    bool pc_increment_handled = false;

    if (opCode == 0x00EE)
    {
        // Return from subroutine. Set the PC to the address at the top of the stack and subtract 1 from the SP.

        // Check whether there is a return address on the stack.
        if (sp_ == 0)
        {
            LOG_DEBUG("Invalid return instruction given, stack is empty.\n");
            return false;
        }

        LOG_DEBUG("Returing from subroutine");
        sp_ -= 1;
        pc_ = stack_.at(sp_);
        stack_.at(sp_) = 0;
        pc_increment_handled = true;
    }
    else if (opCode == 0x00E0)
    {
        LOG_DEBUG("Clearing display.\n");
        for (auto &item : display_)
        {
            item.fill(0);
        }
    }
    else if (family == 0x1)
    {
        // Set PC to NNN.
        pc_ = nnn;
        pc_increment_handled = true;
    }
    else if (family == 0x2)
    {
        // Call subroutine a NNN.
        // Increment the SP and put the current PC value on the top of the stack.
        // Then set the PC to NNN. Generally there is a limit of 16 successive calls.

        // Check whether the stack has space for another return address.
        if (sp_ == stack_.size())
        {
            LOG_DEBUG("Invalid call instruction given, stack is full.\n");
            return false;
        }

        stack_.at(sp_) = pc_ + 2;
        sp_ += 1;
        pc_ = nnn;
        pc_increment_handled = true;
    }
    else if (family == 0x3)
    {
        // Skip the next instruction if register VX is equal to NN.
        if (v_.at(x) == nn)
        {
            pc_ += 2;
        }
    }
    else if (family == 0x4)
    {
        // Skip the next instruction if register VX is not equal to NN.
        if (v_.at(x) != nn)
        {
            pc_ += 2;
        }
    }
    else if (family == 0x5 && n == 0x0)
    {
        // Skip the next instruction if register VX equals VY.
        if (v_.at(x) == v_.at(y))
        {
            pc_ += 2;
        }
    }
    else if (family == 0x6)
    {
        // Load immediate value NN into register VX.
        LOG_DEBUG("Loading V" << x << " with " << nn << "\n");
        v_.at(x) = nn;
        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
    }
    else if (family == 0x7)
    {
        // Add immediate value NN to register VX. Does not effect VF.
        LOG_DEBUG("Adding V" << x << " with " << nn << "\n");
        v_.at(x) += nn;
        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
    }
    else if (family == 0x8 && n == 0x0)
    {
        // Copy the value in register VY into VX
        LOG_DEBUG("Copying V" << y << " into  V" << x << "\n");
        v_.at(x) = v_.at(y);
        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
        LOG_DEBUG("V" << y << ": " << int(v_.at(y)) << "\n");
    }
    else if (family == 0x8 && n == 0x1)
    {
        // Set VX equal to the bitwise OR of the values in VX and VY.
        LOG_DEBUG("Bitwise OR of V" << x << "with  V" << y << "\n");
        v_.at(x) = v_.at(x) | v_.at(y);
        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
        LOG_DEBUG("V" << y << ": " << int(v_.at(y)) << "\n");
    }
    else if (family == 0x8 && n == 0x2)
    {
        // Set VX equal to the bitwise AND of the values in VX and VY.
        LOG_DEBUG("Bitwise AND of V" << x << "with  V" << y << "\n");
        v_.at(x) = v_.at(x) & v_.at(y);
        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
        LOG_DEBUG("V" << y << ": " << int(v_.at(y)) << "\n");
    }
    else if (family == 0x8 && n == 0x3)
    {
        // Set VX equal to the bitwise XOR of the values in VX and VY.
        LOG_DEBUG("Bitwise XOR of V" << x << "with  V" << y << "\n");
        v_.at(x) = v_.at(x) ^ v_.at(y);
        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
        LOG_DEBUG("V" << y << ": " << int(v_.at(y)) << "\n");
    }
    else if (family == 0x8 && n == 0x4)
    {
        // Set VX equal to VX plus VY. In the case of an overflow VF is set to 1. Otherwise 0.
        LOG_DEBUG("Adding V" << x << " with V" << y << "\n");
        std::uint8_t aux = v_.at(x);
        v_.at(x) += v_.at(y);
        if (aux > v_.at(x))
        {
            LOG_DEBUG("V" << x << " overflowen, setting Vf to 1.\n");
            v_.at(0xF) = 1;
        }
        else
        {
            v_.at(0xF) = 0;
        }
        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
        LOG_DEBUG("VF: " << int(v_.at(0xF)) << "\n");
    }
    else if (family == 0x8 && n == 0x5)
    {
        // Set VX equal to VX minus VY. In the case of an underflow VF is set 0. Otherwise 1. (VF = VX > VY)
        LOG_DEBUG("Subtracting V" << x << " with V" << y << "\n");

        std::uint8_t aux = v_.at(x);
        v_.at(x) -= v_.at(y);

        if (aux < v_.at(x))
        {
            LOG_DEBUG("V" << x << " underflowen, setting Vf to 0.\n");
            v_.at(0xF) = 0;
        }
        else
        {
            v_.at(0xF) = 1;
        }

        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
        LOG_DEBUG("Vf: " << int(v_.at(0xF)) << "\n");
    }
    else if (family == 0x8 && n == 0x6)
    {
        // Set VX equal to VX bitshifted right 1. VF is set to the least significant bit of VX prior to the shift.
        // Originally this opcode meant set VX equal to VY bitshifted right 1 but emulators and software seem to ignore VY now.
        LOG_DEBUG("Getting the least significant bit of VX.\n");
        std::uint8_t flag = v_.at(x) & 0x01;

        LOG_DEBUG("Bitshifting V" << x << " right 1 bit.\n");
        v_.at(x) = v_.at(x) >> 1;

        LOG_DEBUG("Assigning the result to VF.\n");
        v_.at(0xF) = flag;

        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
        LOG_DEBUG("Vf: " << int(v_.at(0xF)) << "\n");
    }
    else if (family == 0x8 && n == 0x7)
    {
        // Set VX equal to VY minus VX. VF is set to 1 if VY > VX. Otherwise 0.
        std::uint8_t oldVY = v_.at(y);
        std::uint8_t oldVX = v_.at(x);

        LOG_DEBUG("Subtracting V" << y << " with V" << x << "\n");
        v_.at(x) = oldVY - oldVX;

        if (oldVY >= oldVX)
        {
            v_.at(0xF) = 1;
        }
        else
        {
            LOG_DEBUG("V" << x << " underflow detected, setting Vf to 0.\n");
            v_.at(0xF) = 0;
        }

        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
        LOG_DEBUG("Vf: " << int(v_.at(0xF)) << "\n");
    }
    else if (family == 0x8 && n == 0xE)
    {
        // Set VX equal to VX bitshifted left 1. VF is set to the most significant bit of VX prior to the shift.
        // Originally this opcode meant set VX equal to VY bitshifted left 1 but emulators and software seem to ignore VY now.
        LOG_DEBUG("Getting the most significant bit of VX.\n");
        std::uint8_t flag = (v_.at(x) & 0x80) >> 7;

        LOG_DEBUG("Bitshifting V" << x << " left 1 bit.\n");
        v_.at(x) = v_.at(x) << 1;

        LOG_DEBUG("Assigning the result to VF.\n");
        v_.at(0xF) = flag;

        LOG_DEBUG("V" << x << ": " << int(v_.at(x)) << "\n");
        LOG_DEBUG("Vf: " << int(v_.at(0xF)) << "\n");
    }
    else if (family == 0x9)
    {
        if (n == 0x0)
        {
            if (v_.at(x) != v_.at(y))
            {
                pc_ += 2;
            }
        }
    }
    else if (family == 0xA)
    {
        // Set I equal to NNN.
        LOG_DEBUG("Setting Index: " << nnn << "\n");
        index_ = nnn;
    }
    else if (family == 0xB)
    {
        // Set the PC to NNN plus the value in V0.
        LOG_DEBUG("Setting PC: " << nnn << " + " << int(v_.at(0x0)) << "\n");
        pc_ = nnn + v_.at(0x0);
        pc_increment_handled = true;
    }
    else if (family == 0xC)
    {
        // Set VX equal to a random number ranging from 0 to 255 which is logically anded with NN.
        std::random_device rd;
        std::uniform_int_distribution<int> distribution(0, 255);
        std::uint8_t random_number = distribution(rd);

        LOG_DEBUG(std::dec);
        LOG_DEBUG("Setting V" << x << " to randomly generated number: " << int(random_number) << " ANDed with " << nn << "\n");
        LOG_DEBUG(std::hex);
        v_.at(x) = random_number & nn;
    }
    else if (family == 0xD)
    {
        // Display N-byte sprite starting at memory location I at (VX, VY).
        // Each set bit of XOR-ed with what's already drawn.
        // VF is set to 1 if a collision occurs. 0 otherwise.
        if (index_ + n > memory_.size())
        {
            LOG_DEBUG("Sprite exceeds memory bounds at " << index_ + n - 1 << ", Display not modified.\n");
        }
        else
        {
            LOG_DEBUG("Drawing " << n << "-byte sprite at (" << int(v_.at(x)) << "," << int(v_.at(y)) << ")\n");

            v_.at(0xF) = 0;

            for (std::uint8_t i = 0; i < n; i++)
            {
                std::uint8_t sprite_byte = memory_[index_ + i];

                for (std::uint8_t j = 0; j < 8; j++)
                {
                    std::uint8_t sprite_pixel = (sprite_byte >> (7 - j)) & 1;

                    std::uint8_t screen_y = (v_.at(y) + i) % 32;
                    std::uint8_t screen_x = (v_.at(x) + j) % 64;

                    auto old_pixel = display_[screen_y][screen_x];
                    display_.at(screen_y).at(screen_x) = old_pixel ^ sprite_pixel;

                    if (old_pixel == 1 && sprite_pixel == 1)
                    {
                        v_.at(0xF) = 1;
                    }
                }
            }
        }
    }
    else if (family == 0xE && nn == 0x9E)
    {
        // Skip the following instruction if the key represented by the value in VX is pressed.
        if (v_.at(x) >= keypad_.size())
        {
            LOG_DEBUG("Invalid key given in V" << x << "\n");
        }
        else if (keypad_.at(v_.at(x)))
        {
            LOG_DEBUG("Keypad " << int(v_.at(x)) << " pressed. Skipping next instruction.\n");
            pc_ += 2;
        }
    }
    else if (family == 0xE && nn == 0xA1)
    {
        // Skip the following instruction if the key represented by the value in VX is not pressed.
        if (v_.at(x) >= keypad_.size())
        {
            LOG_DEBUG("Invalid key given in V" << x << "\n");
        }
        else if (!keypad_.at(v_.at(x)))
        {
            LOG_DEBUG("Keypad " << int(v_.at(x)) << " not pressed. Skipping next instruction.\n");
            pc_ += 2;
        }
    }
    else if (family == 0xF && nn == 0x07)
    {
        // Set VX equal to the delay timer.
        LOG_DEBUG("Setting V" << x << " equal to delay timer: " << int(delayTimer_) << "\n");
        v_.at(x) = delayTimer_;
    }
    else if (family == 0xF && nn == 0x0A)
    {
        pc_increment_handled = true;

        if (fx0aState == Fx0aState::Idle)
        {
            fx0aState = Fx0aState::WaitingForPress;
        }
        else if (fx0aState == Fx0aState::Ready && fx0aKey_.has_value())
        {
            v_.at(x) = fx0aKey_.value();
            fx0aKey_.reset();
            pc_increment_handled = false;
            fx0aState = Fx0aState::Idle;
        }
    }
    else if (family == 0xF && nn == 0x15)
    {
        // Set the delay timer DT to VX.
        LOG_DEBUG("Setting delay timer to V" << x << ": " << int(v_.at(x)) << "\n");
        delayTimer_ = v_.at(x);
    }
    else if (family == 0xF && nn == 0x18)
    {
        // Set the sound timer ST to VX.
        LOG_DEBUG("Setting sound timer to V" << x << ": " << int(v_.at(x)) << "\n");
        soundTimer_ = v_.at(x);
    }
    else if (family == 0xF && nn == 0x1E)
    {
        // Add VX to I.
        // VF is set to 1 if I > 0x0FFF. Otherwise set to 0.
        LOG_DEBUG("Adding V" << x << ": " << int(v_.at(x)) << "to Index.\n");

        index_ += v_.at(x);

        if (index_ > 0x0FFF)
        {
            v_.at(0xF) = 1;
        }
        else
        {
            v_.at(0xF) = 0;
        }
    }
    else if (family == 0xF && nn == 0x29)
    {
        // Set I = location of sprite for digit Vx.
        // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
        // Memory location for the digits starts at 0x00 through 0x4F, and they are sorted ascending.
        // Sprites are 8x5 meaning each digit starts at an address multiple of 5.
        std::uint8_t digit = v_.at(x) & 0x0F;

        index_ = digit * 5;
    }
    else if (family == 0xF && nn == 0x33)
    {
        // Convert the value stored in VX to BCD and store the 3 digits at memory location I through I+2.
        // I does not change.
        if (index_ + 2 >= memory_.size())
        {
            LOG_DEBUG("Memory out of bounds.\n");
        }
        else
        {
            LOG_DEBUG("Converting V" << x << ": " << int(v_.at(x)) << " to BCD.\n");

            memory_.at(index_) = v_.at(x) / 100;
            memory_.at(index_ + 1) = (v_.at(x) / 10) % 10;
            memory_.at(index_ + 2) = v_.at(x) % 10;
        }
    }
    else if (family == 0xF && nn == 0x55)
    {
        // Store registers V0 through VX in memory starting at location I.
        // I does not change.
        if (index_ + x >= memory_.size())
        {
            LOG_DEBUG("Memory out of bounds.\n");
        }
        else
        {
            LOG_DEBUG("Copying from registers into memory. Start index: " << index_ << " End index: " << index_ + x << "\n");

            for (std::uint16_t i = 0; i <= x; i++)
            {
                memory_.at(i + index_) = v_.at(i);
            }
        }
    }
    else if (family == 0xF && nn == 0x65)
    {
        // Copy values from memory location I through I + X into registers V0 through VX.
        // I does not change.
        if (index_ + x >= memory_.size())
        {
            LOG_DEBUG("Memory out of bounds.\n");
        }
        else
        {
            LOG_DEBUG("Copying from memory into registers. Start index: " << index_ << " End index: " << index_ + x << "\n");

            for (std::uint16_t i = 0; i <= x; i++)
            {
                v_.at(i) = memory_.at(i + index_);
            }
        }
    }
    else
    {
        LOG_DEBUG("opcode not implemented yet.\n");
    }

    if (!pc_increment_handled)
    {
        pc_ += 2;
    }

    return true;
}

void Chip8::tickTimers()
{
    if (delayTimer_ > 0)
    {
        delayTimer_--;
    }

    if (soundTimer_ > 0)
    {
        soundTimer_--;
    }
}

void Chip8::setKeyState(std::uint8_t key, bool pressed)
{
    if (key >= keypad_.size())
    {
        LOG_DEBUG("Invalid key pressed.\n");
        return;
    }

    bool wasPressed = keypad_.at(key);
    keypad_.at(key) = pressed;

    if (fx0aState == Fx0aState::WaitingForPress && pressed && !wasPressed)
    {
        fx0aKey_ = key;
        fx0aState = Fx0aState::WaitingForRelease;
    }
    if (fx0aState == Fx0aState::WaitingForRelease && !pressed && wasPressed && fx0aKey_ == key)
    {
        fx0aState = Fx0aState::Ready;
    }
}

const std::array<std::array<std::uint8_t, 64>, 32> &Chip8::getDisplay() const
{
    return display_;
}