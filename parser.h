#ifndef ANSI_PARSER_H
#define ANSI_PARSER_H

#include <stdint.h>

// Callback Interface
// parser will call these function pointers whenever it completes a sequence.
typedef struct {
    void (*print)(uint32_t codepoint);
    void (*sgr)(int *params, int n_params);
    void (*cursor_move)(int row, int col);
    void (*erase_display)(int mode);
    void (*erase_line)(int mode);
    void (*cursor_up)(int n);
    void (*cursor_down)(int n);
    void (*cursor_right)(int n);
    void (*cursor_left)(int n);
} TermCallbacks;

// we now pass the callback struct into the parser so it knows where to send events
void parse_byte(char c, TermCallbacks *cb);

#endif