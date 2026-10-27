#include <iostream>
#include <ostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
    //Initialize SDL and Window
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("FENT-8", 1280, 640, 0);
    if (window == nullptr) {
        //End with an error if window doesn't open for whatever reason
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //Create the renderer that draws things to the window
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //Create an event handler so you can quit
    SDL_Event e;
    bool quit = false;

    //Rectangle for testing purposes
    SDL_FRect fentSquare {270, 190, 100, 100};

    //Main Loop
    while (!quit) {
        //Checks if the user has quit or not
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        //Render the FUCKINGGGGG rectangle
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);     //Sets render color with black
        SDL_RenderClear(renderer);                                     //Colors the entire screen black

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);   //green
        SDL_RenderFillRect(renderer, &fentSquare);                     //draw rectangle

        SDL_RenderPresent(renderer);                                   //Draws the shit in the renderer to the window
    }

    //End Program
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}