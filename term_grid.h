#ifndef TERM_GRID_H
#define TERM_GRID_H


#include<stdint.h>
#include "parser.h"

#define MAX_ROWS 50
#define MAX_COLS 100 //legacy stuff, dont ask me bruh

typedef struct{
    uint32_t codepoint; //actual character
    uint8_t fg_color; // foreground color
    uint8_t bg_color; // background color
} Cell;

extern Cell grid[MAX_ROWS][MAX_COLS];

void init_grid();
void dump_grid();

extern TermCallbacks grid_callbacks;

#endif