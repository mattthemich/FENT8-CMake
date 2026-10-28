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
    }

    chip8::~chip8() = default;

    void chip8::emulate_cycle() {
        //todo
    }

    //Memory Operations
    void chip8::load_fonts() {
        //todo
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
                set_pixel(i, j, (mod));
            }
        }
    }
} // chip8