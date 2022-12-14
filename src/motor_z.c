#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

int vz = 0;

int fd_z, fd_z_world;
int log_mz;

fd_set rfds;
struct timeval tv;
int retval;

char * myfifo_z = "/tmp/myfifo_z";

char * myfifo_z_world = "/tmp/myfifo_z_world";

char str_z[2];
char str_pos_z[10];


int pos_z = 0;

int stopped;

void handler_stop(int signal_number)
{
	write(log_mz, "Stop signal received.\n", 23);
	stopped = 1;
	vz = 0;
	char msg[15];
	sprintf(msg, "Vz = %d\n", vz);
	write(log_mz, msg, strlen(msg));
}

void handler_reset (int signal_number)
{
	write(log_mz, "Reset signal received.\n", 24);

	mkfifo(myfifo_z_world, 0666);
	vz = -1;
	while(pos_z != 0)
	{

		if(stopped)
		{
			stopped = 0;
			return;
		}

		pos_z += vz;

		sprintf(str_pos_z,"%d", pos_z);

		fd_z_world = open(myfifo_z_world, O_WRONLY);

		if(fd_z_world == -1){
			write(log_mz, "Error while opening the pipe connected to world.\n", 50);
		}

		if(write(fd_z_world, str_pos_z, strlen(str_pos_z)+1) == -1){
			write(log_mz, "Error while writing in the pipe connected to world.\n", 53);
		}

		if(close(fd_z_world) == -1){
			write(log_mz,"Error while closing the pipe connected to world.\n", 50);
		}

		FD_ZERO(&rfds);
		FD_SET(fd_z, &rfds);
		
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		retval = select(fd_z+1, &rfds, NULL, NULL, &tv);

		char msg[15];

		if (retval < 0){
			write(log_mz, "Select: ", 9);
			write(log_mz, strerror(errno), strlen(strerror(errno)));
			write(log_mz, "\n", 2);
		}
		else if (retval > 0) {
			if(FD_ISSET(fd_z, &rfds)){
		
				if(read(fd_z, str_z, 2) == -1){
					write(log_mz, "Error while reading from the pipe connected to command console\n", 64);
				}

			}
		}
	}
	vz = 0;

}

int main(){

	//open log file for motor z
	log_mz = open("logFiles/motor_z.log", O_WRONLY | O_APPEND | O_CREAT, 0666);  
	ftruncate(log_mz, 0);
	
	mkfifo(myfifo_z, 0666);

	mkfifo(myfifo_z_world, 0666);

	if(signal(SIGUSR1, handler_stop) == SIG_ERR){
		write(log_mz, "Error while receiving stop signal.\n", 36);
		return -1;
	}
	
	if(signal(SIGUSR2, handler_reset) == SIG_ERR){
		write(log_mz, "Error while receiving reset signal.\n", 37);
		return -1;
	}
	
	
	while(1){
		
		//open pipe connected to command console
		fd_z = open(myfifo_z, O_RDONLY | O_NONBLOCK);
		if(fd_z == -1){
			write(log_mz, "Error while opening the pipe in connection to command console\n", 63);
			return -1;
		}
		
		FD_ZERO(&rfds);
		FD_SET(fd_z, &rfds);
		
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		retval = select(fd_z+1, &rfds, NULL, NULL, &tv);

		char msg[15];

		if (retval < 0){
			write(log_mz, "Select: ", 9);
			write(log_mz, strerror(errno), strlen(strerror(errno)));
			write(log_mz, "\n", 2);
		}
		else if (retval > 0) {
			if(FD_ISSET(fd_z, &rfds)){
		
				if(read(fd_z, str_z, 2) == -1){
					write(log_mz, "Error while reading from the pipe connected to command console\n", 64);
					return -1;
				}

				if(strcmp(str_z, "1") == 0){
					vz++;
					sprintf(msg, "Vz = %d\n", vz);
					write(log_mz, msg, strlen(msg));

				}else if(strcmp(str_z, "2") == 0){
					vz--;
					sprintf(msg, "Vz = %d\n", vz);
					write(log_mz, msg, strlen(msg));
					
				}else if(strcmp(str_z, "0") == 0){
					vz = 0;
					sprintf(msg, "Vz = %d\n", vz);
					write(log_mz, msg, strlen(msg));
					
				}
			}
		}

		if(vz != 0){
			pos_z += vz;

			if(pos_z > 9){
				write(log_mz, "Maximum vertical position reached... the program is setting vertical velocity to zero.\n", 88);
				vz = 0;
				pos_z = 10;
			}
			if(pos_z < 0){
				write(log_mz, "Minimum vertical position reached... the program is setting vertical velocity to zero.\n", 88);
				vz = 0;
				pos_z = 0;
			}

			sprintf(str_pos_z,"%d", pos_z);

			fd_z_world = open(myfifo_z_world, O_WRONLY);
			if(fd_z_world == -1){
				write(log_mz, "Error while opening the pipe connected to world.\n", 50);
				return -1;
			}

			if(write(fd_z_world, str_pos_z, strlen(str_pos_z)+1) == -1){
				write(log_mz, "Error while writing in the pipe connected to world.\n", 53);
				return -1;
			}else{
				write(log_mz, "The current vertical position is: ", 35);
				write(log_mz, str_pos_z, strlen(str_pos_z)+1);
				write(log_mz, "\n", 2);
			}

			if(close(fd_z_world) == -1){
				write(log_mz,"Error while closing the pipe connected to world.\n", 50);
				return -1;
			}
		}

		if(close(fd_z) == -1){
			write(log_mz,"Error while closing the pipe connected to command console.\n", 60);
			return -1;
		}
		
	}

	close(log_mz);

	return 0;
}
