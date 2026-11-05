//
// Created by fent on 10/27/26.
//

#include <stdlib.h>
#include <fstream>
#include "chip8.h"
#include <random>

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
        chip8::y_shift = false;
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
        //std::cout << std::hex << chip8::opcode << std::endl;      //Uncomment for opcode debugging

        //Splits opcode into 4 nibbles
        unsigned short nib1 = chip8::opcode >> 12;
        unsigned short nib2 = chip8::opcode >> 8 & 0x000F;
        unsigned short nib3 = chip8::opcode >> 4 & 0x000F;
        unsigned short nib4 = chip8::opcode & 0x000F;

        //Only used for case functions
        bool none_pressed = true;
        unsigned char result;

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
                        if (sp > 0) {
                            sp --;
                        }
                        pc = stack[sp];
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

            //CALL addr
            case 0x02:
                stack[sp] = pc;
                sp++;
                pc = opcode & 0x0FFF;
                break;

            //SE VX, byte
            case 0x03:
                if (V[nib2] == ((nib3 << 4) | nib4)){
                    pc += 2;
                }
                break;

            //SNE VX, byte
            case 0x04:
                if (V[nib2] != ((nib3 << 4) | nib4)){
                    pc += 2;
                }
                break;

            //SE VX, VY
            case 0x05:
                if (V[nib2] == V[nib3]) {
                    pc += 2;
                }
                break;

            //LD VX
            case 0x06:
                V[nib2] = (nib3 << 4 | nib4);
                break;

            //ADD VX, BYTE
            case 0x07:
                V[nib2] = V[nib2] + (nib3 << 4 | nib4);
                break;

            //8xyn cases, there's a lot of these
            case 0x08:
                switch (nib4) {
                    //LD VX, VY
                    case 0x00:
                        V[nib2] = V[nib3];
                        break;

                    //OR VX, VY
                    case 0x01:
                        V[nib2] = V[nib2] | V[nib3];
                        V[15] = 0;
                        break;

                    //AND VX, VY
                    case 0x02:
                        V[nib2] = V[nib2] & V[nib3];
                        V[15] = 0;
                        break;

                    //XOR VX, VY
                    case 0x03:
                        V[nib2] = V[nib2] ^ V[nib3];
                        V[15] = 0;
                        break;

                    //ADD VX, VY
                    case 0x04:
                        result = V[nib2] + V[nib3];

                        if ((short)V[nib2] + (short)V[nib3] > 255) {
                            V[0XF] = 1;
                        }else {
                            V[0XF] = 0;
                        }

                        if (nib2 != 15) V[nib2] = result;

                        break;

                    //SUB VX, VY
                    case 0x05:
                        result = V[nib2] - V[nib3];

                        if (V[nib2] >= V[nib3]) {
                            V[15] = 1;
                        }else {
                            V[15] = 0;
                        }

                        if (nib2 != 15) V[nib2] = result;
                        break;

                    //SHR VX {1, VY}
                    case 0x06:
                        if (y_shift) {
                            V[15] = V[nib3] & 1;
                            if (nib2 != 15) V[nib2] = V[nib3] / 2;
                        }else {
                            V[15] = V[nib2] & 1;
                            if (nib2 != 15) V[nib2] /= 2;
                        }
                        break;

                    //SUBN VX, VY
                    case 0x07:
                        result = V[nib3] - V[nib2];

                        if (V[nib3] >= V[nib2]) {
                            V[15] = 1;
                        }else {
                            V[15] = 0;
                        }

                        if (nib2 != 15) V[nib2] = result;
                        break;

                    //SHL VX {1, VY}
                    case 0x0E:
                        if (y_shift) {
                            V[15] = (V[nib3] & 0b10000000) >> 7;
                            if (nib2 != 15) V[nib2] = V[nib3] * 2;
                        }else {
                            V[15] = (V[nib2] & 0b10000000) >> 7;
                            if (nib2 != 15) V[nib2] *= 2;
                        }
                        break;

                    default:
                        std::cout << "ERROR: Unknown opcode " << std::hex << opcode << "; Ending program...\n";
                        quit_flag = true;
                        break;

                    break;

                }

                break;

            //SNE VX, VY
            case 0x09:
                if (V[nib2] != V[nib3]) {
                    pc += 2;
                }
                break;

            //LD I, ADDR
            case 0x0A:
                I = opcode & 0x0FFF;
                break;

            //JP V0, ADDR
            case 0x0B:
                pc = (opcode & 0x0FFF) + V[0];
                break;

            //RND VX, BYTE
            case 0x0C:
                V[nib2] = ((rd() % 255) & ((char)(nib3 << 4) | nib4));
                break;

            //DRAW
            case 0x0D:
                //Loops through each byte in the sprite
                for (int i = 0; i < nib4; i++) {
                    unsigned char byte = read_mem(I+i);

                    //Loops through each bit in the byte, XOR it with the screen
                    for (int j = 0; j < 8; j++) {
                        unsigned char bit = (byte >> (7 - j)) & 0b00000001;
                        unsigned int draw_x = ((V[nib2] % 64) + j);
                        unsigned int draw_y = ((V[nib3] % 32) + i);
                        unsigned char new_bit = bit ^ screen[(draw_x + (draw_y * 64))];
                        screen[(draw_x + (draw_y * 64))] = new_bit;

                        if (bit == 1 && new_bit == 0) {
                            V[15] = 1;
                        }
                    }

                    draw_flag = true;
                }

                break;

            //Input Calls
            case 0x0E:
                switch ((nib3 << 4) | nib4) {
                    //SKP VX
                    case 0x9E:
                        if (key[V[nib2]] == 1) pc += 2;
                        break;

                    //SKNP VX
                    case 0xA1:
                        if (key[V[nib2]] == 0) pc += 2;
                        break;

                    default:
                        std::cout << "ERROR: Unknown opcode " << std::hex << opcode << "; Ending program...\n";
                        quit_flag = true;
                        break;
                }

                break;

            case 0x0F:
                switch ((nib3 << 4) | nib4) {
                    //LD VX, DT
                    case 0x07:
                        V[nib2] = delay_timer;
                        break;

                    //LD VX, K
                    case 0x0A:
                        for (char i = 0; i < 16; i++) {
                            if (key[i] > 0 && none_pressed == true) {
                                V[nib2] = i;
                                none_pressed = false;
                            }
                        }

                        if (none_pressed) {
                            pc -= 2;
                        }
                        break;

                    //LD DT, VX
                    case 0x15:
                        delay_timer = V[nib2];
                        break;

                    //LD ST, VX
                    case 0x18:
                        sound_timer = V[nib2];
                        break;

                    //ADD I, VX
                    case 0x1E:
                        I += V[nib2];
                        break;

                    //LD F, VX
                    case 0x29:
                        I = FONT_ADDRESS + (5 * V[nib2]);
                        break;

                    //LD B, VX
                    case 0x33:
                        memory[I] = V[nib2] / 100;
                        memory[I+1] = ((V[nib2] - (memory[I] * 100)) / 10);
                        memory[I+2] = V[nib2] - (memory[I] * 100) - (memory[I+1] * 10);
                        break;

                    //LD I, VX
                    case 0x55:
                        for (int i = 0; i < nib2 + 1; i++) {
                            memory[I] = V[i];
                            I++;
                        }
                        break;

                    //LD VX, I
                    case 0x65:
                        for (int i = 0; i < nib2 + 1; i++) {
                            V[i] = memory[I];
                            I++;
                        }
                        break;

                    default:
                        std::cout << "ERROR: Unknown opcode " << std::hex << opcode << "; Ending program...\n";
                        quit_flag = true;
                        break;
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

    /**
     * Determines whether SHR and SHL operates only on VX (false), or on VY and VX (true)
     * @param flag The value for yshift.
     */
    void chip8::set_yshift(bool flag) {
        y_shift = flag;
    }

    //Input Operations
    void chip8::set_key_state(int k, unsigned char val) {
        key[k % 16] = val;
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
        this->draw_flag = flag;
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

    //Timer Operations
    void chip8::decrement_delay() {
        if (delay_timer > 0) {
            //std::cout << "Delay Decremented to " << std::hex << delay_timer << std::endl;
            delay_timer--;
        }
    }

    void chip8::decrement_sound() {
        if (sound_timer > 0) {
            //std::cout << "Sound Decremented to " << std::hex << delay_timer << std::endl;
            sound_timer--;
        }
    }
} // chip8