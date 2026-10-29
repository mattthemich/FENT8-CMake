//
// Created by fent on 10/27/26.
//

#include "chip8.h"

namespace chip8 {
    //General
    chip8::chip8() {
        chip8::opcode = 0;
        chip8::memory[4096] = {0};
        chip8::V[16] = {0};
        chip8::I = 0;
        chip8::pc = 0;
        chip8::screen[64*32] = {0};
        chip8::delay_timer = 0;
        chip8::sound_timer = 0;
        chip8::stack[16] = {0};
        chip8::sp = 0;
        chip8::key[16] = {0};
        chip8::draw_flag = false;
        chip8::c1 = 0xFF6DE36F;
        chip8::c2 = 0xFF244924;
    }

    chip8::~chip8() = default;

    void chip8::emulate_cycle() {
        //todo
    }

    //Memory Operations
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

    void chip8::load_game(char *filepath) {
        //todo
    }

    unsigned char chip8::read_mem(unsigned int addr) {
        //todo
    }

    void chip8::write_mem(unsigned int addr, unsigned char val) {
        //todo
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