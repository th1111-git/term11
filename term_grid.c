#include<stdio.h>
#include<string.h>
#include "term_grid.h"

Cell grid[MAX_ROWS][MAX_COLS];

int cursor_row = 0;
int cursor_col = 0;
uint8_t current_fg_color = 7; // default white
uint8_t current_bg_color = 0; // default black

void init_grid() {
    for (int r = 0; r < MAX_ROWS; r++) {
        for (int c = 0; c < MAX_COLS; c++) {
            grid[r][c].codepoint = ' '; // space character
            grid[r][c].fg_color = current_fg_color;
            grid[r][c].bg_color = current_bg_color;
        }
    }
}

void scroll_up(){
    //move everything up by one row 
    memmove(&grid[0][0], &grid[1][0], sizeof(Cell) * (MAX_ROWS - 1) * MAX_COLS);
    //clear the last row
    for (int c = 0; c < MAX_COLS; c++) {
        grid[MAX_ROWS - 1][c].codepoint = ' '; 
        grid[MAX_ROWS - 1][c].fg_color = current_fg_color;
        grid[MAX_ROWS - 1][c].bg_color = current_bg_color;
    }
}

void grid_print(uint32_t codepoint) {
    if (codepoint == '\r') {
        cursor_col = 0; // Return to start of line
        return;
    }
    if (codepoint == '\n') {
        cursor_row++;   // Move down a line
        if (cursor_row >= MAX_ROWS) {
            scroll_up();
            cursor_row = MAX_ROWS - 1;
        }
        return;
    }
    if (codepoint == '\b' || codepoint == 0x7F) { // Backspace
        if (cursor_col > 0) cursor_col--;
        return;
    }

    if (codepoint < 0x20) {
        return;
    }

    grid[cursor_row][cursor_col].codepoint = codepoint;
    grid[cursor_row][cursor_col].fg_color = current_fg_color;
    grid[cursor_row][cursor_col].bg_color = current_bg_color;
    cursor_col++;
    if (cursor_col >= MAX_COLS) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= MAX_ROWS) {
            scroll_up();
            cursor_row = MAX_ROWS - 1;
        }
    }
}


void grid_sgr(int *params, int n_params) {
    for (int i = 0; i < n_params; i++) {
        int p = params[i];
        if (p == 0) { // Reset
            current_fg_color = 7; // default white
            current_bg_color = 0; // default black
        } else if (p >= 30 && p <= 37) { // Set foreground color
            current_fg_color = p - 30;
        } else if (p >= 40 && p <= 47) { // Set background color
            current_bg_color = p - 40;
        }
    }
}

void grid_cursor_move(int row, int col) {
    // ANSI codes are 1-indexed, our array is 0-indexed
    cursor_row = row > 0 ? row - 1 : 0;
    cursor_col = col > 0 ? col - 1 : 0;
    
    // clamp to bounds to prevent out-of-bounds crashes
    if (cursor_row >= MAX_ROWS) cursor_row = MAX_ROWS - 1;
    if (cursor_col >= MAX_COLS) cursor_col = MAX_COLS - 1;
}

void grid_erase_display(int mode) {
    if (mode == 2) { // clear entire screen
        init_grid();
        cursor_row = 0;
        cursor_col = 0;
    }
}

void grid_erase_line(int mode) {
    if (mode == 0) {
        // clear from cursor to the end of the line
        for (int c = cursor_col; c < MAX_COLS; c++) {
            grid[cursor_row][c].codepoint = ' ';
            grid[cursor_row][c].fg_color = current_fg_color;
            grid[cursor_row][c].bg_color = current_bg_color;
        }
    }
}

void dump_grid() {
    for (int r = 0; r < MAX_ROWS; r++) {
        for (int c = 0; c < MAX_COLS; c++) {
            putchar(grid[r][c].codepoint);
        }
        printf("\r\n");
    }
}

void grid_cursor_up(int n) {
    cursor_row -= n;
    if (cursor_row < 0) cursor_row = 0;
}

void grid_cursor_down(int n) {
    cursor_row += n;
    if (cursor_row >= MAX_ROWS) cursor_row = MAX_ROWS - 1;
}

void grid_cursor_right(int n) {
    cursor_col += n;
    if (cursor_col >= MAX_COLS) cursor_col = MAX_COLS - 1;
}

void grid_cursor_left(int n) {
    cursor_col -= n;
    if (cursor_col < 0) cursor_col = 0;
}

TermCallbacks grid_callbacks = {
    .print = grid_print,
    .sgr = grid_sgr,
    .cursor_move = grid_cursor_move,
    .erase_display = grid_erase_display,
    .erase_line = grid_erase_line,
    .cursor_up = grid_cursor_up,
    .cursor_down = grid_cursor_down,
    .cursor_right = grid_cursor_right,
    .cursor_left = grid_cursor_left
};