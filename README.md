# FENT8-CMake
A CHIP8 Emulator using SDL3 and CMake

## Building
Currently, this project only supports building for Linux, but I'll eventually get around to adding directions for Windows and MacOS.

### Linux
1. Clone this project somewhere on your computer with
```
git clone https://github.com/mattthemich/FENT8-CMake.git
```

2. Go into the FENT8 folder, and clone SDL3 into Vendored/SDL
```
cd FENT8-CMake
git clone https://github.com/libsdl-org/SDL.git vendored/SDL
```

3. Build the project
```
cmake -S . -B build
cmake --build build
```

## Usage
You start FENT8 from the command line, along with the path to the game you want to open.
```
./fent8 GAME_PATH="/path/to/game.ch8"
```

There are also a few other launch arguments you can use.
- RES = How large the game window will be scaled up. Default is 20. Example: RES=15
- Y_SHIFT = Determines if bit-shifting operations use the Y register. 0 means it uses VX, anything greater means it will use VY. 1 is the default. Example: Y_SHIFT=0
- PALETTE = Reads in palette data from a text file. Data in the file should be formatted as 0xAARRGGBB,0xAARRGGBB. Example: PALETTE="path/to/file.txt"
- VBLANK = Determines if draw opcodes wait for vblank to operate. This is more accurate, but much slower. Default is 0 (do not wait). Example: VBLANK=1
