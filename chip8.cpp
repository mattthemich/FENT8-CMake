//
// Created by fent on 10/27/26.
//

#include <fstream>
#include "chip8.h"

#include <iostream>

namespace chip8 {
    //General

    /**
     * The default constructor for the CHIP-8 object. Needs no arguments.
     */
    chip8::chip8() {
        chip8::opcode = 0;
        chip8::memory[4096] = {0};
        chip8::V[16] = {0};
        chip8::I = 0;
        chip8::pc = START_ADDRESS;
        chip8::screen[64*32] = {0};
        chip8::delay_timer = 0;
        chip8::sound_timer = 0;
        chip8::stack[16] = {0};
        chip8::sp = 0;
        chip8::key[16] = {0};
        chip8::draw_flag = false;
        chip8::c2 = 0xFF6DE36F;
        chip8::c1 = 0xFF244924;
        chip8::emu_speed = 700;
        chip8::quit_flag = false;
    }

    /**
     * Default destructor. Currently does nothing.
     */
    chip8::~chip8() = default;

    /**
     * Emulates one cycle of the CHIP-8.
     *
     * This includes grabbing an opcode from the ram at the PC, and running certain commands based on that opcode.
     *
     * Afterwards it will increase the PC by one and end the cycle.
     */
    void chip8::emulate_cycle() {
        //Grabs the current opcode from memory
        chip8::opcode = chip8::read_mem(chip8::pc) << 8 | chip8::read_mem(chip8::pc + 1);
        chip8::pc += 2;

        //Splits opcode into 4 nibbles
        unsigned short nib1 = chip8::opcode >> 12;
        unsigned short nib2 = chip8::opcode >> 8 & 0x000F;
        unsigned short nib3 = chip8::opcode >> 4 & 0x000F;
        unsigned short nib4 = chip8::opcode & 0x000F;

        switch (nib1) {
            //Handles CLS and RET (and SYSCALL, but that is unimplemented)
            case 0x00:
                switch (opcode) {
                    //CLS
                    case 0x00E0:
                        for (int i = 0; i < (64*32); i++) {
                            screen[i] = 0x00;
                        }
                        draw_flag = true;
                        break;

                    //RET
                    case 0x00EE:
                        pc = stack[sp];
                        if (sp > 0) {
                            sp --;
                        }
                        break;

                    default:
                        std::cout << "ERROR: Unknown opcode " << std::hex << opcode << "; Ending program...\n";
                        quit_flag = true;
                        break;
                }
                break;

            //Literally just the JMP command
            case 0x01:
                pc = opcode & 0x0FFF;
                break;

            //LD VX
            case 0x06:
                V[nib2] = (nib3 << 4 | nib4);
                break;

            //ADD VX, BYTE
            case 0x07:
                V[nib2] = V[nib2] + (nib3 << 4 | nib4);
                break;

            //LD I, ADDR
            case 0x0A:
                I = opcode & 0x0FFF;
                break;

            //DRAW
            case 0x0D:
                //Loops through each byte in the sprite
                for (int i = 0; i < nib4; i++) {
                    unsigned char byte = read_mem(I+i);

                    //Loops through each bit in the byte, XOR it with the screen
                    for (int j = 0; j < 8; j++) {
                        unsigned char bit = (byte >> (7 - j)) & 0b00000001;
                        unsigned int draw_x = (V[nib2] + j);
                        unsigned int draw_y = (V[nib3] + i);
                        unsigned char new_bit = bit ^ screen[((draw_x % 64) + ((draw_y % 32) * 64))];
                        screen[((draw_x % 64) + ((draw_y % 32) * 64))] = new_bit;

                        if (bit == 1 && new_bit == 0) {
                            V[15] = 1;
                        }
                    }

                    draw_flag = true;
                }

                break;

            default:
                std::cout << "ERROR: Unknown Opcode " << std::hex << opcode << "; Ending program...\n";
                quit_flag = true;
                break;
        }
    }

    /**
     *
     * @return true only if an illegal operation runs
     */
    bool chip8::get_quit_flag() {
        return quit_flag;
    }

    /**
     * Returns the emulation speed of the CHIP-8 object.
     * @return The amount of cycles that should run in 1 second as an unsigned integer.
     */
    unsigned int chip8::get_emu_speed() {
        return chip8::emu_speed;
    }

    //Memory Operations
    /**
     * Loads the default font into memory.
     */
    void chip8::load_fonts() {
        unsigned char fonts[80] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0,   //0
            0x20, 0x60, 0x20, 0x20, 0x70,   //1
            0xF0, 0x10, 0xF0, 0x80, 0xF0,   //2
            0xF0, 0x10, 0xF0, 0x10, 0xF0,   //3
            0x90, 0x90, 0xF0, 0x10, 0x10,   //4
            0xF0, 0x80, 0xF0, 0x10, 0xF0,   //5
            0xF0, 0x80, 0xF0, 0x90, 0xF0,   //6
            0xF0, 0x10, 0x20, 0x40, 0x40,   //7
            0xF0, 0x90, 0xF0, 0x90, 0xF0,   //8
            0xF0, 0x90, 0xF0, 0x10, 0xF0,   //9
            0xF0, 0x90, 0xF0, 0x90, 0x90,   //A
            0xE0, 0x90, 0xE0, 0x90, 0xE0,   //B
            0xF0, 0x80, 0x80, 0x80, 0xF0,   //C
            0xE0, 0x90, 0x90, 0x90, 0xE0,   //D
            0xF0, 0x80, 0xF0, 0x80, 0xF0,   //E
            0xF0, 0x80, 0xF0, 0x80, 0x80    //F
        };

        for (int i = 0; i < 80; i++) {
            chip8::write_mem(0x050 + i, fonts[i]);
        }
    }

    /**
     * Loads a .ch8 file into memory starting at 0x200.
     * @param filepath Should be the filepath leading to a .ch8 file.
     */
    void chip8::load_game(char *filepath) {
        //Open the ifstream for a read-only file
        std::ifstream game(filepath, std::ios::binary | std::ios::ate);

        if (game.is_open()) {
            //Get the size of the file, and make a character buffer to store the data
            //It grabs the size of the file by setting the stream position all the way at the end of the file,
            //and then storing that position in the size variable.
            std::streampos size = game.tellg();
            char* buffer = new char[size];

            //After that, we then
            game.seekg(0, std::ios::beg);
            game.read(buffer, size);
            game.close();

            //Move buffer data into memory
            for (int i = 0; i < size; i++) {
                memory[0x200 + i] = buffer[i];
            }

            //Delete the buffer
            delete[] buffer;

        }else {
            std::cout << "ERROR: game file could not be opened.\n";
        }
    }

    /**
     * Returns a character value from the CHIP-8's memory.
     * @param addr The address in memory to grab the value from
     * @return The char value kept inside addr.
     */
    unsigned char chip8::read_mem(unsigned int addr) {
        return memory[addr];
    }

    void chip8::write_mem(unsigned int addr, unsigned char val) {
        memory[addr] = val;
    }

    //Screen Operations
    void chip8::draw_sprite() {
        //todo
    }

    unsigned int chip8::get_color1() {
        return chip8::c1;
    }

    unsigned int chip8::get_color2() {
        return chip8::c2;
    }

    bool chip8::get_draw_flag() {
        return draw_flag;
    }

    unsigned char chip8::get_pixel(int x, int y) {
        return chip8::screen[x + (y*64)];
    }

    void chip8::set_draw_flag(bool flag) {
        chip8::draw_flag = flag;
    }

    void chip8::set_pixel(int x, int y, unsigned char val) {
        if (screen[x+(y*64)] != val) {
            screen[x+(y*64)] = val;
            draw_flag = true;
        }
    }

    void chip8::test_pattern() {
        for (int j = 0; j < 32; j++) {
            for (int i = 0; i < 64; i++) {
                unsigned char mod = ((i%2)+(j%2))%2;
                set_pixel(i, j, mod);
            }
        }
    }
} // chip8