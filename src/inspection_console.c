#include "./../include/inspection_utilities.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

int main(int argc, char const *argv[])
{

    int log_ic;
	//open log file for inspection console
	log_ic = open("logFiles/inspection.log", O_WRONLY | O_APPEND | O_CREAT, 0666);  
	ftruncate(log_ic, 0);

    int fd_final;

    char * myfifo_final = "/tmp/myfifo_final";
	mkfifo(myfifo_final, 0666);

    char pos_final[20];
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // End-effector coordinates
    float ee_x, ee_y;

    //retrieve the pid of the processes motor_x and motor_z
    char line_x[80];
    FILE *cmd_x = popen("pidof motor_x", "r");

    char line_z[80];
    FILE *cmd_z = popen("pidof motor_z", "r");

    fgets(line_x, 80, cmd_x);
    pid_t pid_x = strtoul(line_x, NULL, 10);

    fgets(line_z, 80, cmd_z);
    pid_t pid_z = strtoul(line_z, NULL, 10);

    pclose(cmd_x);
    pclose(cmd_z);

    // Initialize User Interface 
    init_console_ui();

    // Infinite loop
    while(TRUE)
	{	

        fd_final = open(myfifo_final, O_RDONLY);
        if(fd_final== -1){
			write(log_ic, "Error while opening the pipe connected to world\n", 49);
            return -1;
		}

        if(read(fd_final, pos_final, strlen(pos_final)+1) == -1){
            write(log_ic, "Error while opening the pipe connected to world.\n", 50);
            return -1;
        }

        sscanf(pos_final, "%f,%f", &ee_x, &ee_y);

        
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

                // STOP button pressed
                if(check_button_pressed(stp_button, &event)) {
				   
				    if(kill(pid_x, SIGUSR1) == -1){
                        write(log_ic, "Error while sending stop signal to motor x.\n", 45);
                        return -1;
                    }else{
                        write(log_ic, "Stop signal succesfully sent to motor x.\n", 42);
                    }

				    if(kill(pid_z, SIGUSR1) == -1){
                        write(log_ic, "Error while sending stop signal to motor z.\n", 45);
                        return -1;
                    }else{
                        write(log_ic, "Stop signal succesfully sent to motor z.\n", 42);
                    }

                }

                // RESET button pressed
                else if(check_button_pressed(rst_button, &event)) {
                    
                    if(kill(pid_x, SIGUSR2) == -1){
                        write(log_ic, "Error while sending reset signal to motor x.\n", 46);
                        return -1;
                    }else{
                        write(log_ic, "Reset signal succesfully sent to motor x.\n", 43);
                    }

				    if(kill(pid_z, SIGUSR2) == -1){
                        write(log_ic, "Error while sending reset signal to motor z.\n", 46);
                        return -1;
                    }else{
                        write(log_ic, "Reset signal succesfully sent to motor z.\n", 43);
                    }

                }
            }
        }
        
        // To be commented in final version...
        /*switch (cmd)
        {
            case KEY_LEFT:
                ee_x--;
                break;
            case KEY_RIGHT:
                ee_x++;
                break;
            case KEY_UP:
                ee_y--;
                break;
            case KEY_DOWN:
                ee_y++;
                break;
            default:
                break;
        }*/
        
        // Update UI
        update_console_ui(&ee_x, &ee_y);
        if(close(fd_final) == -1){
            write(log_ic, "Error while closing the pipe connected to world.\n", 50);
            return -1;
        }
	}

    // Terminate
    close(log_ic);
    endwin();
    return 0;
}
