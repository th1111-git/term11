#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pty.h>
#include<termios.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <errno.h>

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

int main(){
    int master_fd;

    pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);
    if(pid < 0){
        perror("forkpty error");
        exit(1);
    }
    else if (pid == 0){
        // Child
        execlp("/bin/bash", "bash", NULL);
        perror("execlp error");
        exit(1);
    }

    // Parent
    enable_raw_mode();
    char buffer[256];
    fd_set read_fds;

    while(1){
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(master_fd, &read_fds);

        //wait
        int max_fd = (STDIN_FILENO > master_fd) ? STDIN_FILENO : master_fd;
        if(select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0){
            if(errno == EINTR) continue; // retry signal interrupt
            break;
        }
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
                    parse_byte(buffer[i]);
                }            
            }

            else{
                //EOF
                break;
            }
        }
    }

    int status;
    waitpid(pid, &status, 0);
    return 0;
}