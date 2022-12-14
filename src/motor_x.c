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

int vx = 0;

int fd_x, fd_x_world;
int log_mx;
	
int pos_x = 0;

fd_set rfds;
struct timeval tv;
int retval;

char * myfifo_x = "/tmp/myfifo_x";

char * myfifo_x_world = "/tmp/myfifo_x_world";

int stopped = 0;

char str_x[2];
char str_pos_x[10];

void handler_stop (int signal_number)
{
	write(log_mx, "Stop signal received.\n", 23);
	stopped = 1;
	vx = 0;
	char msg[15];
	sprintf(msg, "Vx = %d\n", vx);
	write(log_mx, msg, strlen(msg));
	
} 

void handler_reset (int signal_number)
{
	write(log_mx, "Reset signal received.\n", 24);

	mkfifo(myfifo_x_world, 0666);
	vx = -1;
	while(pos_x > 0)
	{
		if(stopped)
		{
			stopped = 0;
			return;
		}
		pos_x += vx;

		sprintf(str_pos_x,"%d", pos_x);

		fd_x_world = open(myfifo_x_world, O_WRONLY);

		if(fd_x_world == -1){
			write(log_mx, "Error while opening the pipe connected to world.\n", 50);
		}

		if(write(fd_x_world, str_pos_x, strlen(str_pos_x)+1) == -1){
			write(log_mx, "Error while writing in the pipe connected to world.\n", 53);
		}

		if(close(fd_x_world) == -1){
			write(log_mx,"Error while closing the pipe connected to world.\n", 50);
		}

		FD_ZERO(&rfds);
		FD_SET(fd_x, &rfds);
		
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		retval = select(fd_x+1, &rfds, NULL, NULL, &tv);
		
		char msg[15];

		if (retval < 0){
			write(log_mx, "Select: ", 9);
			write(log_mx, strerror(errno), strlen(strerror(errno)));
			write(log_mx, "\n", 2);
		}
		else if (retval > 0) {
			if(FD_ISSET(fd_x, &rfds)){
				
				if(read(fd_x, str_x, 2) == -1){
					write(log_mx, "Error while reading from the pipe connected to command console\n", 64);
				}
	
			}
			
		}

	}


	
	vx = 0;

}

int main(){

	//open log file for motor x
	log_mx = open("logFiles/motor_x.log", O_WRONLY | O_APPEND | O_CREAT, 0666);  
	ftruncate(log_mx, 0);

	mkfifo(myfifo_x, 0666);

	mkfifo(myfifo_x_world, 0666);
	
	if(signal(SIGUSR1, handler_stop) == SIG_ERR){
		write(log_mx, "Error while receiving stop signal.\n", 36);
		return -1;
	}
	
	if(signal(SIGUSR2, handler_reset) == SIG_ERR){
		write(log_mx, "Error while receiving reset signal.\n", 37);
		return -1;
	}
	
	while(1){
	
		//open pipe connected to command console
		fd_x = open(myfifo_x, O_RDONLY | O_NONBLOCK);
		if(fd_x == -1){
			write(log_mx, "Error while opening the pipe in connection to command console\n", 63);
			return -1;
		}
		
		
		FD_ZERO(&rfds);
		FD_SET(fd_x, &rfds);
		
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		retval = select(fd_x+1, &rfds, NULL, NULL, &tv);
		
		char msg[15];

		if (retval < 0){
			write(log_mx, "Select: ", 9);
			write(log_mx, strerror(errno), strlen(strerror(errno)));
			write(log_mx, "\n", 2);
		}
		else if (retval > 0) {
			if(FD_ISSET(fd_x, &rfds)){
				
				if(read(fd_x, str_x, 2) == -1){
					write(log_mx, "Error while reading from the pipe connected to command console\n", 64);
					return -1;
				}
	
				if(strcmp(str_x, "1") == 0){
					vx++;
					sprintf(msg, "Vz = %d\n", vx);
					write(log_mx, msg, strlen(msg));
				}else if(strcmp(str_x, "2") == 0){
					vx--;
					sprintf(msg, "Vz = %d\n", vx);
					write(log_mx, msg, strlen(msg));
				}else if(strcmp(str_x, "0") == 0){
					vx = 0;
					sprintf(msg, "Vz = %d\n", vx);
					write(log_mx, msg, strlen(msg));
				}
			}
			
		}


		if(vx != 0){
			
			pos_x += vx;

			if(pos_x > 39){
				write(log_mx, "Maximum horizontal position reached... the program is setting vertical velocity to zero.\n", 90);
				vx = 0;
				pos_x = 40;
			}
			if(pos_x < 0){
				write(log_mx, "Minimum horizontal position reached... the program is setting vertical velocity to zero.\n", 90);
				vx = 0;
				pos_x = 0;
			}

			sprintf(str_pos_x,"%d", pos_x);

			fd_x_world = open(myfifo_x_world, O_WRONLY);

			if(fd_x_world == -1){
				write(log_mx, "Error while opening the pipe connected to world.\n", 50);
				return -1;
			}

			if(write(fd_x_world, str_pos_x, strlen(str_pos_x)+1) == -1){
				write(log_mx, "Error while writing in the pipe connected to world.\n", 53);
				return -1;
			}else{
				write(log_mx, "The current horizontal position is: ", 37);
				write(log_mx, str_pos_x, strlen(str_pos_x)+1);
				write(log_mx, "\n", 2);
			}

			if(close(fd_x_world) == -1){
				write(log_mx,"Error while closing the pipe connected to world.\n", 50);
				return -1;
			}
		}
		
		if(close(fd_x) == -1){
			write(log_mx,"Error while closing the pipe connected to command console.\n", 60);
			return -1;
		}

		
		
	}

	close(log_mx);
	return 0;
}
