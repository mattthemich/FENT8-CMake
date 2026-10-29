#include <iostream>
#include <ostream>
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
static int res;

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
    //Check if an argument for the screen size was passed
    res = 20;

    if (argc == 2) {
        try {
            res = std::stoi(argv[1]);
            std::cout << "Using scaling number " << res << "\n";
        }catch (const std::invalid_argument& errorCode) {
            std::cerr << "Error: invalid scaling size. Defaulting to 640 x 320.\n";
            res = 20;
        }
    }else if (argc > 2) {
        std::cout << "Too many arguments\n";
        return 1;
    }

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
    bool quit = false;

    //CHIP8 Screen Test
    fent8.test_pattern();

    //Main Loop
    while (!quit) {
        //Checks if the user has quit or not
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        //Render the FUCKINGGGGG screen
        if (fent8.get_draw_flag()) {
            render();
            update();
        }
    }

    //End Program
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}