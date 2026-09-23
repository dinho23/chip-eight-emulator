# CHIP-8 Emulator

A CHIP-8 emulator written in modern C++ using SDL3 for graphics, keyboard input, and audio.

I built this project primarily as a C++ refresher and as an exercise in low-level programming concepts such as instruction decoding, memory management, timing, graphics, input handling, and audio generation.

The emulator can currently run real CHIP-8 games such as Pong with working graphics, keyboard input, timers, and sound.

## Features

- 4 KB CHIP-8 memory
- 16 general-purpose 8-bit registers
- Index register and program counter
- 16-level call stack
- CHIP-8 font set
- ROM loading
- Instruction fetch/decode/execute cycle
- 64×32 monochrome display
- Sprite drawing and collision detection
- CHIP-8 keypad emulation
- 60 Hz delay and sound timers
- Configurable CPU execution rate
- SDL3 rendering
- SDL3 keyboard input
- Generated square-wave audio
- Independent CPU, timer, and rendering clocks
- Sleep-based main loop to avoid busy waiting

## Controls

The CHIP-8 keypad:

```text
1 2 3 C
4 5 6 D
7 8 9 E
A 0 B F
```

is mapped to the keyboard as:

```text
1 2 3 4
Q W E R
A S D F
Z X C V
```

Mapping:

| Keyboard | CHIP-8 |
|----------|--------|
| 1 | 1 |
| 2 | 2 |
| 3 | 3 |
| 4 | C |
| Q | 4 |
| W | 5 |
| E | 6 |
| R | D |
| A | 7 |
| S | 8 |
| D | 9 |
| F | E |
| Z | A |
| X | 0 |
| C | B |
| V | F |

## Building

### Requirements

- C++17 compatible compiler
- CMake
- SDL3

On macOS with Homebrew:

```bash
brew install cmake sdl3
```

Configure and build:

```bash
cmake -S . -B build
cmake --build build
```

## Running

Pass a CHIP-8 ROM as the first argument:

```bash
./build/chip_eight path/to/rom.ch8
```

For example:

```bash
./build/chip_eight roms/pong.ch8
```

ROM files are not included in this repository.

## Architecture

The emulator core is kept mostly independent from SDL.

`Chip8` owns the virtual machine state:

- memory
- registers
- stack
- program counter
- index register
- timers
- framebuffer
- keypad state

The main loop is responsible for the platform side of the emulator:

```text
SDL events
    ↓
keyboard state
    ↓
CHIP-8 CPU execution (~700 Hz)
    ↓
timers (60 Hz)
    ↓
framebuffer rendering (60 Hz)
    ↓
SDL audio
```

CPU execution, timers, and rendering are scheduled independently using `std::chrono`.

The thread sleeps until the next scheduled CPU, timer, or rendering event instead of continuously busy-waiting.

## Display

CHIP-8 uses a 64×32 monochrome framebuffer.

Internally it is represented as:

```text
display[y][x]
```

Each CHIP-8 pixel is scaled before being rendered through SDL.

Sprite drawing uses XOR behavior as required by CHIP-8. `VF` is set when drawing removes an already-set pixel.

## Audio

CHIP-8 has a simple sound timer.

While the sound timer is non-zero, the emulator generates a square wave and feeds it to an SDL audio stream.

Current audio configuration:

```text
Sample rate: 48000 Hz
Channels:    Mono
Format:      Signed 16-bit PCM
Tone:        ~500 Hz square wave
```

The audio queue is kept deliberately small to reduce latency when the sound timer reaches zero.

## Compatibility / Quirks

CHIP-8 implementations historically differ in the behavior of several instructions.

The current emulator uses the following behavior:

### Shift instructions

`8XY6` and `8XYE` shift `VX` directly.

```text
VX = VX >> 1
VX = VX << 1
```

`VY` is not used as the source operand.

### FX55 / FX65

The index register `I` is left unchanged after register store/load operations.

### BNNN

Jump instructions use `V0`:

```text
PC = NNN + V0
```

### Drawing

Sprites wrap around the edges of the 64×32 framebuffer rather than being clipped.

### Logic instructions

`8XY1`, `8XY2`, and `8XY3` do not reset `VF`.

### Display timing

Drawing instructions do not wait for vertical blank.

These choices correspond to a modern CHIP-8-style configuration rather than attempting to exactly reproduce the behavior of the original COSMAC VIP interpreter.

## Implemented Instructions

The standard CHIP-8 instruction set is implemented, including:

```text
00E0    CLS
00EE    RET

1NNN    JP addr
2NNN    CALL addr

3XNN    SE VX, byte
4XNN    SNE VX, byte
5XY0    SE VX, VY

6XNN    LD VX, byte
7XNN    ADD VX, byte

8XY0    LD VX, VY
8XY1    OR VX, VY
8XY2    AND VX, VY
8XY3    XOR VX, VY
8XY4    ADD VX, VY
8XY5    SUB VX, VY
8XY6    SHR VX
8XY7    SUBN VX, VY
8XYE    SHL VX

9XY0    SNE VX, VY

ANNN    LD I, addr
BNNN    JP V0, addr
CXNN    RND VX, byte
DXYN    DRW VX, VY, nibble

EX9E    SKP VX
EXA1    SKNP VX

FX07    LD VX, DT
FX0A    LD VX, K
FX15    LD DT, VX
FX18    LD ST, VX
FX1E    ADD I, VX
FX29    LD F, VX
FX33    LD B, VX
FX55    LD [I], VX
FX65    LD VX, [I]
```

## Testing

Development started with small hand-written binary ROMs used to test individual instructions.

The emulator is also being tested using the Timendus CHIP-8 test suite.

Current results:

- Corax+ opcode test: passing
- Flags test: passing
- Quirks test: behavior verified and documented
- Keypad test: passing
- Beep test: passing

## Project Structure

```text
chip-eight-emulator/
├── CMakeLists.txt
├── include/
│   └── Chip8.h
├── roms/
└── src/
    ├── Chip8.cpp
    └── main.cpp
```

## What I Learned

This project was mainly built as a practical C++ refresher.

Some of the topics covered while building it include:

- fixed-width integer types
- binary file handling
- bitwise operations
- memory addressing
- instruction decoding
- integer overflow and underflow
- register aliasing
- references and const correctness
- `std::array`
- `std::vector`
- `std::optional`
- `std::chrono`
- thread sleeping and scheduling
- SDL3 rendering
- keyboard event handling
- PCM audio generation
- emulator compatibility quirks
- CMake project configuration

One particularly interesting part of the implementation was handling instructions where `VF` can simultaneously be an operand and the destination flag. This required calculating instruction results from the original register values before modifying emulator state.

## Status

The emulator is functional and can run playable CHIP-8 games.

Remaining work is mostly focused on compatibility testing, cleanup, error handling, and small architectural improvements rather than implementing major missing functionality.
