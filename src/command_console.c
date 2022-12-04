#include "./../include/command_utilities.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    
    int fd_x, fd_z;
    
    //create pipes
    char * myfifo_x = "/tmp/myfifo_x";
    mkfifo(myfifo_x, 0666);
    
    char * myfifo_z = "/tmp/myfifo_z";
    mkfifo(myfifo_z, 0666);
    
    //create messages
    char incr_str[] = "1";
    char decr_str[] = "2";
    char stop_str[] = "0";   
    

    // Initialize User Interface 
    init_console_ui();
    
    // Infinite loop
    while(TRUE)
	{	
        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();

        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {

                // Vx-- button pressed
                if(check_button_pressed(vx_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    //write on the pipe connected to motor x
                    fd_x = open(myfifo_x, O_WRONLY);
                    write(fd_x, decr_str, strlen(decr_str)+1);
                    close(fd_x);
                    
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    //write on the pipe connected to motor x
                    fd_x = open(myfifo_x, O_WRONLY);
                    write(fd_x, incr_str, strlen(incr_str)+1);
                    close(fd_x);
                    
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    //write on the pipe connected to motor x
                    fd_x = open(myfifo_x, O_WRONLY);
                    write(fd_x, stop_str, strlen(stop_str)+1);
                    close(fd_x);
                    
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    //write on the pipe connected to motor z
                    fd_z = open(myfifo_z, O_WRONLY);
                    write(fd_z, decr_str, strlen(decr_str)+1);
                    close(fd_z);
                    
                    
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    //write on the pipe connected to motor z
                    fd_z = open(myfifo_z, O_WRONLY);
                    write(fd_z, incr_str, strlen(incr_str)+1);
                    close(fd_z);
                    
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    //write on the pipe connected to motor z
                    fd_z = open(myfifo_z, O_WRONLY);
                    write(fd_z, stop_str, strlen(stop_str)+1);
                    close(fd_z);
                    
                }
            }
        }

        refresh();
	}

    // Terminate
    endwin();
    return 0;
}
