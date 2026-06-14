#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pty.h>
#include<termios.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

#include "parser.h"
#include "term_grid.h"

struct termios orig_termios;

void disable_raw_mode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode(){
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    cfmakeraw(&raw);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void update_terminal_size(int pty_fd, int rows, int cols) {
    struct winsize ws;
    ws.ws_row = rows;
    ws.ws_col = cols;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;
    ioctl(pty_fd, TIOCSWINSZ, &ws);
}

int main(){
    int master_fd;

    pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);
    if(pid < 0){
        perror("forkpty error");
        exit(1);
    }
    else if (pid == 0){
        // Child
        setenv("TERM", "xterm-256color", 1);
        execlp("/bin/bash", "bash", NULL);
        perror("execlp error");
        exit(1);
    }

    // Parent
    enable_raw_mode();

    update_terminal_size(master_fd, MAX_ROWS, MAX_COLS); // tell bash it's 80x24!
    char buffer[256];
    fd_set read_fds;

    struct timeval timeout;

    init_grid();
    while(1){
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(master_fd, &read_fds);

        timeout.tv_sec = 0;
        timeout.tv_usec = 10000; // 10ms        

        int max_fd = (STDIN_FILENO > master_fd) ? STDIN_FILENO : master_fd;

        int ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if(ready < 0){
            if(errno == EINTR) continue; // retry signal interrupt
            break;
        }

        if(ready >0){
                //user typed something
            if(FD_ISSET(STDIN_FILENO, &read_fds)){
                ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer));
                if(n > 0){
                    write(master_fd, buffer, n);
                }
            }
            
            //bash output
            if(FD_ISSET(master_fd, &read_fds)){
                ssize_t bytes_read = read(master_fd, buffer, sizeof(buffer));
                if(bytes_read > 0){
                    for (int i = 0; i < bytes_read; i++) {
                        parse_byte(buffer[i], &grid_callbacks); // Use our grid callbacks
                    }
                }

                else{
                    //EOF
                    break;
                }
            }
        }
       
    }

    int status;
    waitpid(pid, &status, 0);
    return 0;
}