#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <ranges>
#include <stdexcept>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "chip8.h"

//Vars needed before main
chip8::chip8 fent8;
unsigned int* gFrameBuffer;
SDL_Texture* gSDLTexture;
SDL_Window* window;
SDL_Renderer* renderer;
static int gDone;
bool no_games;

//CMD arguments
static int res;                     //First Argument, sets resolution of screen; RES=20
static char* game_path;             //filepath of game; GAME_PATH="/home/user/Downloads/game name.ch8"
static bool y_shift = true;         //Sets how bit-shift ops work
static bool vblank = false;         //Makes draw opcode wait for VBlank; limits to 60 sprites/second

void set_arguments(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        //Try to delimit the string by the = sign
        std::string orig_arg = argv[i];
        std::string delimiter = "=";
        std::string arg_type = orig_arg.substr(0, orig_arg.find(delimiter));
        for (char &c : arg_type) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }

        //If the specific variable is specified
        if (arg_type == "RES") {
            try {
                int scale = std::stoi(orig_arg.substr(orig_arg.find(delimiter) + 1, orig_arg.size() - (arg_type.size() + 1)));
                res = scale;
            }catch (const std::invalid_argument& error) {
                std::cerr << "Error: invalid scale size. Defaulting to 640 x 320.\n";
                res = 20;
            }catch (const std::out_of_range& error) {
                std::cerr << "Error: invalid scale size. Defaulting to 640 x 320.\n";
                res = 20;
            }
        }else if (arg_type == "GAME_PATH") {
            std::string path = orig_arg.substr(orig_arg.find(delimiter) + 1, orig_arg.size() - (arg_type.size() + 1));
            if (std::filesystem::exists(path)) {
                game_path = new char[path.length()];
                strcpy(game_path, path.c_str());
                no_games = false;
            }else {
                no_games = true;
            }
        }else if (arg_type == "Y_SHIFT") {
            try {
                int shift = std::stoi(orig_arg.substr(orig_arg.find(delimiter) + 1, orig_arg.size() - (arg_type.size() + 1)));
                if (shift > 0) {
                    y_shift = true;
                }else {
                    y_shift = false;
                }
            }catch (const std::invalid_argument& error) {
                std::cerr << "Error: invalid y_shift value. Defaulting to on.\n";
            }catch (const std::out_of_range& error) {
                std::cerr << "Error: invalid y_shift value. Defaulting to on.\n";
            }
        }else if (arg_type == "PALETTE") {
            std::string path = orig_arg.substr(orig_arg.find(delimiter) + 1, orig_arg.size() - (arg_type.size() + 1));
            if (std::filesystem::exists(path)) {
                //Read in palette data
                std::ifstream file(path);

                if (file.is_open()) {
                    //Read string into whole variable
                    std::string whole;
                    getline(file, whole);
                    file.close();

                    //Split string into 2 parts, extract unsigned ints from string
                    std::string delimiter = ",";
                    std::string s1 = whole.substr(0, whole.find(delimiter));
                    std::string s2 = whole.substr(whole.find(delimiter) + 1, whole.size() - (s1.size() + 1));

                    fent8.set_palette(std::stoul(s1, nullptr, 16), std::stoul(s2, nullptr, 16));
                }
            }
        }else if (arg_type == "VBLANK") {
            try {
                unsigned int i = std::stoi(orig_arg.substr(orig_arg.find(delimiter) + 1, orig_arg.size() - (arg_type.size() + 1)));
                if (i > 0) vblank = true; else vblank = false;
            }catch (const std::invalid_argument& error) {
                std::cerr << "Error: invalid scale size. Defaulting to 640 x 320.\n";
                res = 20;
            }catch (const std::out_of_range& error) {
                std::cerr << "Error: invalid scale size. Defaulting to 640 x 320.\n";
                res = 20;
            }
        }
    }
}

//Rendering functions
void update() {
    char* pix;
    int pitch;

    /*
     *This section is complicated. Basically, we lock the texture for writing only, which also gives us the
     *pointer to the locked pixels(pix) and the width of 1 row in those pixels(pitch). After locking it, we run a
     *for loop that copies our framebuffer into the texture, 4 rows at a time. From there, we unlock the texture
     *(which copies it into VRAM), render the texture to the screen, and then show the result of the render.
     */
    SDL_LockTexture(gSDLTexture, nullptr, (void**)&pix, &pitch);
    for (int i = 0, sp = 0, dp = 0; i < 32; i++, dp += 64, sp += pitch)
        memcpy(pix + sp, gFrameBuffer + dp, 64*4);

    SDL_UnlockTexture(gSDLTexture);
    SDL_RenderTexture(renderer, gSDLTexture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    SDL_Delay(1);
}

void render() {
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            unsigned char pixel = fent8.get_pixel(j, i);
            unsigned int output = fent8.get_color1();
            if (pixel > 0x00) {
                output = fent8.get_color2();
            }

            gFrameBuffer[j + (i*64)] = output;
        }
    }
}

//The main
int main(int argc, char* argv[]) {
    //Only heck if an argument for the screen size was passed
    res = 20;
    game_path = nullptr;

    set_arguments(argc, argv);

    //Initialize SDL and Window
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("FENT-8", 64*res, 32*res, 0);
    if (window == nullptr) {
        //End with an error if window doesn't open for whatever reason
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //Create the renderer that draws things to the window
    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int WINDOW_WIDTH = 64;
    int WINDOW_HEIGHT = 32;

    gFrameBuffer = new unsigned int[WINDOW_WIDTH * WINDOW_HEIGHT];
    gSDLTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
                                    WINDOW_WIDTH, WINDOW_HEIGHT);
    SDL_SetTextureScaleMode(gSDLTexture, SDL_SCALEMODE_PIXELART);

    //Create an event handler so you can quit
    SDL_Event e;
    bool quit = no_games;

    //Load Chip8 Game
    if (!no_games) {
        fent8.load_game(game_path);
        fent8.set_yshift(y_shift);
    }else {
        std::cout << "No valid game path specified. Exiting...\n";
        quit = true;
    }

    //Main Loop
    while (!quit) {
        //Checks inputs
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }else if (e.type == SDL_EVENT_KEY_DOWN) {
                switch (e.key.key) {
                    case SDLK_1:
                        fent8.set_key_state(0x01, 1);
                        //std::cout << "Key 1 Pressed!" << std::endl;
                        break;

                    case SDLK_2:
                        fent8.set_key_state(0x02, 1);
                        //std::cout << "Key 2 Pressed!" << std::endl;
                        break;

                    case SDLK_3:
                        fent8.set_key_state(0x03, 1);
                        //std::cout << "Key 3 Pressed!" << std::endl;
                        break;

                    case SDLK_4:
                        fent8.set_key_state(0x0C, 1);
                        //std::cout << "Key C Pressed!" << std::endl;
                        break;

                    case SDLK_Q:
                        fent8.set_key_state(0x04, 1);
                        //std::cout << "Key 4 Pressed!" << std::endl;
                        break;

                    case SDLK_W:
                        fent8.set_key_state(0x05, 1);
                        //std::cout << "Key 5 Pressed!" << std::endl;
                        break;

                    case SDLK_E:
                        fent8.set_key_state(0x06, 1);
                        //std::cout << "Key 6 Pressed!" << std::endl;
                        break;

                    case SDLK_R:
                        fent8.set_key_state(0x0D, 1);
                        //std::cout << "Key D Pressed!" << std::endl;
                        break;

                    case SDLK_A:
                        fent8.set_key_state(0x07, 1);
                        //std::cout << "Key 7 Pressed!" << std::endl;
                        break;

                    case SDLK_S:
                        fent8.set_key_state(0x08, 1);
                        //std::cout << "Key 8 Pressed!" << std::endl;
                        break;

                    case SDLK_D:
                        fent8.set_key_state(0x09, 1);
                        //std::cout << "Key 9 Pressed!" << std::endl;
                        break;

                    case SDLK_F:
                        fent8.set_key_state(0x0E, 1);
                        //std::cout << "Key E Pressed!" << std::endl;
                        break;

                    case SDLK_Z:
                        fent8.set_key_state(0x0A, 1);
                        //std::cout << "Key A Pressed!" << std::endl;
                        break;

                    case SDLK_X:
                        fent8.set_key_state(0x00, 1);
                        //std::cout << "Key 0 Pressed!" << std::endl;
                        break;

                    case SDLK_C:
                        fent8.set_key_state(0x0B, 1);
                        //std::cout << "Key B Pressed!" << std::endl;
                        break;

                    case SDLK_V:
                        fent8.set_key_state(0x0F, 1);
                        //std::cout << "Key F Pressed!" << std::endl;
                        break;

                    default:
                        break;
                }
            }else if (e.type == SDL_EVENT_KEY_UP) {
                switch (e.key.key) {
                    case SDLK_1:
                        fent8.set_key_state(0x01, 0);
                        //std::cout << "Key 1 Released!" << std::endl;
                        break;

                    case SDLK_2:
                        fent8.set_key_state(0x02, 0);
                        //std::cout << "Key 2 Released!" << std::endl;
                        break;

                    case SDLK_3:
                        fent8.set_key_state(0x03, 0);
                        //std::cout << "Key 3 Released!" << std::endl;
                        break;

                    case SDLK_4:
                        fent8.set_key_state(0x0C, 0);
                        //std::cout << "Key C Released!" << std::endl;
                        break;

                    case SDLK_Q:
                        fent8.set_key_state(0x04, 0);
                        //std::cout << "Key 4 Released!" << std::endl;
                        break;

                    case SDLK_W:
                        fent8.set_key_state(0x05, 0);
                        //std::cout << "Key 5 Released!" << std::endl;
                        break;

                    case SDLK_E:
                        fent8.set_key_state(0x06, 0);
                        //std::cout << "Key 6 Released!" << std::endl;
                        break;

                    case SDLK_R:
                        fent8.set_key_state(0x0D, 0);
                        //std::cout << "Key D Released!" << std::endl;
                        break;

                    case SDLK_A:
                        fent8.set_key_state(0x07, 0);
                        //std::cout << "Key 7 Released!" << std::endl;
                        break;

                    case SDLK_S:
                        fent8.set_key_state(0x08, 0);
                        //std::cout << "Key 8 Released!" << std::endl;
                        break;

                    case SDLK_D:
                        fent8.set_key_state(0x09, 0);
                        //std::cout << "Key 9 Released!" << std::endl;
                        break;

                    case SDLK_F:
                        fent8.set_key_state(0x0E, 0);
                        //std::cout << "Key E Released!" << std::endl;
                        break;

                    case SDLK_Z:
                        fent8.set_key_state(0x0A, 0);
                        //std::cout << "Key A Released!" << std::endl;
                        break;

                    case SDLK_X:
                        fent8.set_key_state(0x00, 0);
                        //std::cout << "Key 0 Released!" << std::endl;
                        break;

                    case SDLK_C:
                        fent8.set_key_state(0x0B, 0);
                        //std::cout << "Key B Released!" << std::endl;
                        break;

                    case SDLK_V:
                        fent8.set_key_state(0x0F, 0);
                        //std::cout << "Key F Released!" << std::endl;
                        break;

                    default:
                        break;
                }
            }
        }

        //Slows the emulation down to ~11 operations per frame (if ops per second is 700)
        Uint64 cycle_start = SDL_GetTicks();
        for (int i = 0; i < fent8.get_emu_speed() / 60; i++) {
            fent8.emulate_cycle();
            if (fent8.get_quit_flag()) break;
            if (vblank && fent8.get_draw_flag()) break;
        }
        Uint64 current_ticks = SDL_GetTicks();
        while ((current_ticks - cycle_start) < 1000 / 60) {
            current_ticks = SDL_GetTicks();
        }
        fent8.decrement_delay();
        fent8.decrement_sound();

        if (fent8.get_quit_flag()) {
            quit = true;
        }

        //Render the FUCKINGGGGG screen
        if (fent8.get_draw_flag()) {
            render();
            update();
            fent8.set_draw_flag(false);
        }
    }

    //End Program
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    fent8.~chip8();
    delete[] game_path;

    return 0;
}