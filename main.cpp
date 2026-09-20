#include <iostream>
#include <Chip8.h>

int main(int argc, char *argv[])
{
    Chip8 chip8;
    std::cout << "Read:" << argc << " arguments.\n";

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

    if (chip8.loadRom(argv[1]))
    {
        chip8.cycle();
        chip8.cycle();
        chip8.cycle();
    }

    return 0;
}