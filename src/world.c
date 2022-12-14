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

double addError(int p){
    
    int rand_int = rand() % 5000;
	double rand_double = (double)rand_int / 100000.0;

	if (rand() % 2) 
    	rand_double = -rand_double;

	return p+rand_double;


}

int main(){
	
	int log_wr;
	//open log file for world
	log_wr = open("logFiles/world.log", O_WRONLY | O_APPEND | O_CREAT, 0666);  
	ftruncate(log_wr, 0);

    fd_set rfds;
	struct timeval tv;
	int retval;

    int fd_x_world, fd_z_world, fd_final;

    char * myfifo_x_world = "/tmp/myfifo_x_world";
	mkfifo(myfifo_x_world, 0666);

	char * myfifo_z_world = "/tmp/myfifo_z_world";
	mkfifo(myfifo_z_world, 0666);

	char * myfifo_final = "/tmp/myfifo_final";
	mkfifo(myfifo_final, 0666);

    char pos_x[3];
	char pos_z[3];

	char pos_final[20];

    int px;
	int pz;
	int c=0;

    double real_px, real_pz;

	

    while(1){
	
		//open pipe connected to command console
		fd_x_world = open(myfifo_x_world, O_RDONLY | O_NONBLOCK);
		if(fd_x_world == -1){
			write(log_wr, "Error while opening the pipe connected to motor x.\n", 52);
			return -1;
		}
		fd_z_world = open(myfifo_z_world, O_RDONLY | O_NONBLOCK);
		if(fd_z_world == -1){
			write(log_wr, "Error while opening the pipe connected to motor z.\n", 52);
			return -1;
		}

		FD_ZERO(&rfds);
		FD_SET(fd_x_world, &rfds);
		FD_SET(fd_z_world, &rfds);
		
		tv.tv_sec = 0;
		tv.tv_usec = 50000;
		
		int max = -1;
		if(fd_x_world < fd_z_world){
			max = fd_z_world;
		}
		else
			max = fd_x_world;
		
		char msg[50];

		retval = select(max+1, &rfds, NULL, NULL, &tv);
		
		
		if (retval < 0){
			write(log_wr, "Select: ", 9);
			write(log_wr, strerror(errno), strlen(strerror(errno)));
			write(log_wr, "\n", 2);
		}
		else if (retval > 0) {
			if(FD_ISSET(fd_x_world, &rfds) && FD_ISSET(fd_z_world, &rfds)){

				if(rand() % 2){

					if(read(fd_x_world, pos_x, 3) == -1){
						write(log_wr, "Error while reading from the pipe connected to motor x\n", 56);
						return -1;
					}
                	px = atoi(pos_x);

                	real_px = addError(px);
					sprintf(msg, "Current real horizontal position: %f\n", real_px);
					write(log_wr, msg, strlen(msg));
				
				}
				else{

					if(read(fd_z_world, pos_z, 3) == -1){
						write(log_wr, "Error while reading from the pipe connected to motor z\n", 56);
						return -1;
					}

                	pz = atoi(pos_z);

                	real_pz = addError(pz);
					sprintf(msg, "Current real vertical position: %f\n", real_pz);
					write(log_wr, msg, strlen(msg));

				}
			
			}else if(FD_ISSET(fd_x_world, &rfds)){

				if(read(fd_x_world, pos_x, 3) == -1){
					write(log_wr, "Error while reading from the pipe connected to motor x\n", 56);
					return -1;
				}
				px = atoi(pos_x);

				real_px = addError(px);
				sprintf(msg, "Current real horizontal position: %f\n", real_px);
				write(log_wr, msg, strlen(msg));

			}else if(FD_ISSET(fd_z_world, &rfds)){

				if(read(fd_z_world, pos_z, 3) == -1){
					write(log_wr, "Error while reading from the pipe connected to motor z\n", 56);
					return -1;
				}

				pz = atoi(pos_z);

				real_pz = addError(pz);
				sprintf(msg, "Current real vertical position: %f\n", real_pz);
				write(log_wr, msg, strlen(msg));

			}
			
		}
		
		fd_final = open(myfifo_final, O_WRONLY);
		if(fd_final == -1){
			write(log_wr, "Error while opening the pipe connected to inspection console.\n", 63);
			return -1;
		}

		sprintf(pos_final, "%f,%f", real_px, real_pz);

		if(write(fd_final, pos_final, strlen(pos_final)+1) == -1){
			write(log_wr,"Error while writing in the pipe connected to inspection console.\n", 66);
			return -1;
		}
		
		if(close(fd_final) == -1){
			write(log_wr,"Error while closing the pipe connected to inspection console.\n", 63);
			return -1;
		}
		if(close(fd_x_world) == -1){
			write(log_wr,"Error while closing the pipe connected to motor x.\n", 52);
			return -1;
		}
		if(close(fd_z_world) == -1){
			write(log_wr,"Error while closing the pipe connected to motor z.\n", 52);
			return -1;
		}
		
	}

	close(log_wr);
	return 0;

}