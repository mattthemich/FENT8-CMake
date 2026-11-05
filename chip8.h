#ifndef FENT8_CHIP8_H
#define FENT8_CHIP8_H
#include <random>

namespace chip8 {
    class chip8 {
    public:
        //General
        chip8();                            //Initializes the chip8 emulator
        ~chip8();                           //Obliterate the chip8
        void emulate_cycle();               //Emulates one cycle of the CPU
        unsigned int get_emu_speed();       //Get the speed for the
        bool get_quit_flag();               //Gets value of bool flag
        void set_yshift(bool flag);

        //Input
        void set_key_state(int key, unsigned char val);

        //Memory operations
        void load_fonts();                                      //Loads the fonts into memory
        void load_game(char* filepath);                         //Loads the rom from the filepath into memory
        unsigned char read_mem(unsigned int addr);              //Retrieves a value from memory
        void write_mem(unsigned int addr, unsigned char value); //Writes a value into memory

        //Screen Operations
        void draw_sprite();                                     //Draws a sprite to the screen
        unsigned int get_color1();                              //Returns c1
        unsigned int get_color2();                              //Returns c2
        bool get_draw_flag();                                   //read bitch
        unsigned char get_pixel(int x, int y);                  //Grab a pixel from the virtual screen
        void set_draw_flag(bool flag);                          //peruse the text female canine
        void set_palette(unsigned int color1, unsigned int color2); //Set the palette for the screen
        void set_pixel(int x, int y, unsigned char val);        //DEBUG ONLY: Sets 1 pixel to a certain value
        void test_pattern();            //DEBUG ONLY: Makes a test pattern for the display

        //Timer Operations
        void decrement_delay();         //Decreases the delay timer by 1
        void decrement_sound();         //Decreases the sound timer by 1
    private:
        //Consts
        const unsigned int FONT_ADDRESS = 0x050;
        const unsigned int START_ADDRESS = 0x200;

        //Variables
        unsigned short opcode;          //Stores the current opcode
        unsigned char memory[4096];     //The 4KB of memory
        unsigned char V[16];            //Registers V0-VE and the carry flag (VF)
        unsigned short I;               //Index Register
        unsigned short pc;              //Program counter
        unsigned char screen[64*32];    //The simulated screen of the CHIP8
        unsigned char delay_timer;      //Counts down to 0 at 60hz
        unsigned char sound_timer;      //Makes a sound when it hits 0 (also 60hz)
        unsigned short stack[16];       //Stack used for subroutines and jumps
        unsigned short sp;              //Shows which part of the stack to go to
        unsigned char key[16];          //Array used for keyboard input
        /*
         *  1   2   3   C           1   2   3   4           0   1   2   3
         *  4   5   6   D           q   w   e   r           4   5   6   7
         *  7   8   9   E   ->      a   s   d   f   ->      8   9   A   B
         *  A   0   B   F           z   x   c   v           C   D   E   F
         */
        bool draw_flag;                 //Set to true if the window should be updated
        unsigned int c1;                //Color 1 for the palette
        unsigned int c2;                //Color 2 for the palette
        unsigned int emu_speed;         //CPU Cycles per second (default is 700 hz)
        bool quit_flag;                 //Set to true if an illegal operation is ran
        std::random_device rd;          //Used for randomness
        bool y_shift;                   //Determines the behavior of the 8xy6 and 8xyE opcodes
    };
} // chip8

#endif //FENT8_CHIP8_H
