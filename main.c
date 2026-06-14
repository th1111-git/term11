//well well well

#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pty.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <stdbool.h>


#include "parser.h"
#include "term_grid.h"

#define FONT_WIDTH 10
#define FONT_HEIGHT 20

SDL_Color colors[16] = {
    {0, 0, 0, 255},       // Black
    {128, 0, 0, 255},     // Red
    {0, 128, 0, 255},     // Green
    {128, 128, 0, 255},   // Yellow
    {0, 0, 128, 255},     // Blue
    {128, 0, 128, 255},   // Magenta
    {0, 128, 128, 255},   // Cyan
    {192, 192, 192, 255}, // White
    {128, 128, 128, 255}, // Bright Black (Gray)
    {255, 0, 0, 255},     // Bright Red
    {0, 255, 0, 255},     // Bright Green
    {255, 255, 0, 255},   // Bright Yellow
    {0, 0, 255, 255},     // Bright Blue
    {255, 0, 255, 255},   // Bright Magenta
    {0, 255, 255, 255},   // Bright Cyan
    {255, 255, 255, 255}   // Bright White
};


void update_terminal_size(int pty_fd, int rows, int cols) {
    struct winsize ws;
    //set window size
    ws.ws_col = cols;
    ws.ws_row = rows;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;
    if(ioctl(pty_fd, TIOCSWINSZ, &ws) == -1) {
        perror("ioctl error");
    }

}


int main(){
    //pty setup
    int master_fd;
    pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);
    if(pid == 0){
        setenv("TERM", "xterm-256color", 1);
        execlp("/bin/bash", "/bin/bash", NULL);
        exit(1);
    }

    update_terminal_size(master_fd, MAX_ROWS, MAX_COLS);
    init_grid();

    //SDL setup

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Term11",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        MAX_COLS * FONT_WIDTH, MAX_ROWS * FONT_HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("JetBrainsMono-Medium.ttf", 16);

    SDL_StartTextInput();
    bool running = true;
    char buf[256];
    SDL_Event event;

    while(running){
        //poll keyboard events
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false;
            }

            else if (event.type == SDL_TEXTINPUT){
                write(master_fd, event.text.text, strlen(event.text.text));
            }

            else if(event.type == SDL_KEYDOWN){
                char c;

                // check if the control key is used
                if (event.key.keysym.mod & KMOD_CTRL) {
                    SDL_Keycode sym = event.key.keysym.sym;
                    
                    // convert the letter into ascii control code
                    if (sym >= SDLK_a && sym <= SDLK_z) {
                        c = sym - 'a' + 1;
                        write(master_fd, &c, 1);
                    }
                }
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        c = '\n';
                        write(master_fd, &c, 1);
                        break;
                    case SDLK_BACKSPACE:
                        c = '\x7f';
                        write(master_fd, &c, 1);
                        break;
                    case SDLK_TAB:
                        c = '\t';
                        write(master_fd, &c, 1);
                        break;
                    case SDLK_ESCAPE:
                        c = '\x1b'; 
                        write(master_fd, &c, 1);
                        break;

                    //arrow keys
                    case SDLK_UP:
                            write(master_fd, "\x1b[A", 3);
                            break;
                        case SDLK_DOWN:
                            write(master_fd, "\x1b[B", 3);
                            break;
                        case SDLK_RIGHT:
                            write(master_fd, "\x1b[C", 3);
                            break;
                        case SDLK_LEFT:
                            write(master_fd, "\x1b[D", 3);
                            break;
                }
            }

        }

        // read from pty

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(master_fd, &read_fds);
        struct timeval timeout = {0,0};

        if(select(master_fd+1, &read_fds, NULL, NULL, &timeout)>0){
            ssize_t bytes = read(master_fd, buf, sizeof(buf)-1);
            if(bytes > 0){
                for(int i=0; i<bytes;i++){
                    parse_byte(buf[i], &grid_callbacks);
                }
                // Process the received data
            }

            else{
                running = false;
            }
        }

        // render

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);


        for(int r=0; r<MAX_ROWS;r++){
            for(int c=0; c<MAX_COLS;c++){
                uint32_t codepoint = grid[r][c].codepoint;
                if(codepoint != 0 && codepoint != ' '){
                    char str[2] = {(char)codepoint, '\0'};

                    SDL_Color fg = colors[grid[r][c].fg_color];

                    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, str, fg);
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

                    SDL_Rect dest = {c*FONT_WIDTH, r*FONT_HEIGHT, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, NULL, &dest);

                    SDL_FreeSurface(surface);
                    SDL_DestroyTexture(texture);

                }

            }
        }

        SDL_RenderPresent(renderer);
    }

    //cleanup
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}