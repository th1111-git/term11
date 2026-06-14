#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include "parser.h"

typedef enum{
    STATE_NORMAL,
    STATE_ESC,
    STATE_CSI,
    STATE_OSC
} ParserState;

ParserState current_state = STATE_NORMAL;
int params[16];
int param_count = 0;
int current_param = 0;
int has_digit = 0; // tracks if the user actually typed a number


void parse_byte(char c, TermCallbacks *cb){

    if(!cb) return; // safety check for null pointer

    switch (current_state) {
        
        case STATE_NORMAL:
            if (c == '\x1b') {
                current_state = STATE_ESC;
            } else {
                if(cb->print) cb->print((uint32_t)c); // send the character to the callback
            }
            break;

        case STATE_ESC:
            if (c == '[') {
                current_state = STATE_CSI;
                param_count = 0;
                current_param = 0;
                has_digit = 0;

            } 
            else if (c == ']') {
                current_state = STATE_OSC;
            }


            else {
                // If it's a different escape sequence, ignore and reset
                current_state = STATE_NORMAL; 
            }
            break;

        case STATE_CSI:
            if (isdigit(c)) {
                // Build the number (e.g., '3' then '4' -> 34)
                current_param = (current_param * 10) + (c - '0');
                has_digit = 1;
            } 
            else if (c == ';') {
                // Parameter is done, save it to the array
                params[param_count++] = has_digit ? current_param : 0;
                current_param = 0;
                has_digit = 0;
            } 
            else if (c >= 0x40 && c <= 0x7E) {
                // We hit a letter! The sequence is finished.
                
                // Save the very last parameter
                if (has_digit) {
                    params[param_count++] = current_param;
                } else if (param_count == 0 && c == 'm') {
                    // Edge case: bash sometimes sends just "ESC[m" 
                    // which implicitly means "ESC[0m" (Reset)
                    params[param_count++] = 0;
                }

                if(c == 'm'){
                    if(cb->sgr) cb->sgr(params, param_count); // send SGR event to callback
                }
                else if(c == 'H' || c == 'f'){
                    // Safely extract row and col, defaulting to 1 if missing or zero
                    int row = (param_count > 0 && params[0] > 0) ? params[0] : 1;
                    int col = (param_count > 1 && params[1] > 0) ? params[1] : 1;
                    
                    if(cb->cursor_move) cb->cursor_move(row, col); 
                }

                else if (c == 'A') {
                    int n = (param_count > 0 && params[0] > 0) ? params[0] : 1;
                    if(cb->cursor_up) cb->cursor_up(n);
                }
                else if (c == 'B') {
                    int n = (param_count > 0 && params[0] > 0) ? params[0] : 1;
                    if(cb->cursor_down) cb->cursor_down(n);
                }
                else if (c == 'C') {
                    int n = (param_count > 0 && params[0] > 0) ? params[0] : 1;
                    if(cb->cursor_right) cb->cursor_right(n);
                }
                else if (c == 'D') {
                    int n = (param_count > 0 && params[0] > 0) ? params[0] : 1;
                    if(cb->cursor_left) cb->cursor_left(n);
                }
                else if(c == 'J'){
                    // Default to mode 0 if no parameters were given
                    int mode = (param_count > 0) ? params[0] : 0;
                    
                    if(cb->erase_display) cb->erase_display(mode); 
                }
                else if(c == 'K'){ 
                    int mode = (param_count > 0) ? params[0] : 0;
                    if(cb->erase_line) cb->erase_line(mode);
                }
                              
                current_state = STATE_NORMAL; // reset for next text
            }
            break;

        case STATE_OSC:
            if (c == '\x07') { // BEL character
                current_state = STATE_NORMAL; // reset for next text
            }
            break;
    }
}