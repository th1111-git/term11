#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main(int argc, char *argv[]) {
    //initialize SDL and TTF
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    if(TTF_Init() < 0) {
        fprintf(stderr, "Could not initialize SDL_ttf: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Renderer test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if(!window){
        printf("Window error %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer){
        printf("Renderer error %s\n", SDL_GetError());
        return 1;
    }

    //load font
    TTF_Font* font = TTF_OpenFont("JetBrainsMono-Medium.ttf", 24);

    if(!font){
        printf("Font error %s\n", TTF_GetError());
        return 1;
    }

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* textsurface = TTF_RenderText_Solid(font, "Renderer active!", white);
    SDL_Texture* texttexture = SDL_CreateTextureFromSurface(renderer, textsurface);

    SDL_Rect destrect = {100, 100, textsurface->w, textsurface->h};
    SDL_FreeSurface(textsurface);


    bool running = true;
    SDL_Event event;
    while(running){

        //x button event
        while(SDL_PollEvent(&event) != 0){
            if(event.type == SDL_QUIT){
                running = false;
            }
        }

        //black background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        //draw the text surface
        SDL_RenderCopy(renderer, texttexture, NULL, &destrect);

        //display it
        SDL_RenderPresent(renderer);
    }

    //cleanup
    SDL_DestroyTexture(texttexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}