#include "./../include/command_utilities.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    
    int fd_x, fd_z;
    int log_command;
    
    //create pipes
    char * myfifo_x = "/tmp/myfifo_x";
    mkfifo(myfifo_x, 0666);
    
    char * myfifo_z = "/tmp/myfifo_z";
    mkfifo(myfifo_z, 0666);
    
    //create messages
    char incr_str[] = "1";
    char decr_str[] = "2";
    char stop_str[] = "0";

    //open log file for command console
    log_command = open("logFiles/command.log", O_WRONLY | O_APPEND | O_CREAT, 0666); 
    ftruncate(log_command, 0);

    fd_x = open(myfifo_x, O_WRONLY);
    if(fd_x == -1){
        write(log_command, "Error while opening the pipe in connection to motor x.\n", 56);
        return -1;
    }  

    fd_z = open(myfifo_z, O_WRONLY);
    if(fd_z == -1){
        write(log_command, "Error while opening the pipe in connection to motor z.\n", 56);
        return -1;
    }
    

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
                    
                    //write on the pipe connected to motor x
                    
                    
                    if(write(fd_x, decr_str, strlen(decr_str)+1) == -1){
                        write(log_command, "Error while writing in the pipe in connection to motor x.\n", 59);
                        return -1;
                    }else{
                        write(log_command, "Vx-- button pressed.\n", 22);
                    }
                    
                    
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    
                    
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    refresh();
                    
                    //write on the pipe connected to motor x
                    
                    if(write(fd_x, incr_str, strlen(incr_str)+1) == -1){
                        write(log_command, "Error while writing in the pipe in connection to motor x.\n", 59);
                        return -1;
                    }else{
                        write(log_command, "Vx++ button pressed.\n", 22);
                    }
                    
                    
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    
                    
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    refresh();
                    
                    //write on the pipe connected to motor x
                    
                    if(write(fd_x, stop_str, strlen(stop_str)+1) == -1){
                        write(log_command, "Error while writing in the pipe in connection to motor x.\n", 59);
                        return -1;
                    }else{
                        write(log_command, "Vx stop button pressed.\n", 25);
                    }
                    
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    
                    
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    refresh();
                    
                    //write on the pipe connected to motor z
                    
                    if(write(fd_z, decr_str, strlen(decr_str)+1) == -1){
                        write(log_command, "Error while writing in the pipe in connection to motor z.\n", 59);
                        return -1;
                    }else{
                        write(log_command, "Vz-- button pressed.\n", 22);
                    }
                    
                    
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    
                    
                    
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    refresh();
                    
                    //write on the pipe connected to motor z
                   
                    if(write(fd_z, incr_str, strlen(incr_str)+1) == -1){
                        write(log_command, "Error while writing in the pipe in connection to motor z.\n", 59);
                        return -1;
                    }else{
                        write(log_command, "Vz++ button pressed.\n", 22);
                    }
                    
                    
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    
                    
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    refresh();
                    
                    //write on the pipe connected to motor z
                    
                    if(write(fd_z, stop_str, strlen(stop_str)+1) == -1){
                        write(log_command, "Error while writing in the pipe in connection to motor z.\n", 59);
                        return -1;
                    }else{
                        write(log_command, "Vz stop button pressed.\n", 25);
                    }
                    
                    
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    
                    
                }
            }
        }

        refresh();
	}
    if(close(fd_x) == -1){
        write(log_command, "Error while closing the pipe in connection to motor x.\n", 56);
        return -1;
    }

    if(close(fd_z) == -1){
        write(log_command, "Error while closing the pipe in connection to motor z.\n", 56);
        return -1;
    }

    close(log_command);

    // Terminate
    endwin();
    return 0;
}
