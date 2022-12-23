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

// addError adds a random error value between -0.05 and 0.05 to the given position value
double addError(int p){
    
    // generate a random integer between 0 and 4999
    int rand_int = rand() % 5000;
    	
    	// convert the integer to a double and divide it by 100000.0 to get a value between 0 and 0.04999
	double rand_double = (double)rand_int / 100000.0;

	// if the result of rand() % 2 is 1, negate the random value
	if (rand() % 2) 
    	rand_double = -rand_double;
	
	// return the original position value with the random error added
	return p+rand_double;


}

int main(){
	
	int log_wr;
	//open log file for world
	log_wr = open("logFiles/world.log", O_WRONLY | O_APPEND | O_CREAT, 0666);  
	ftruncate(log_wr, 0); // Truncate log file to zero length

    	// Set up file descriptor set and timeval struct for select function
    	fd_set rfds;
	struct timeval tv;
	int retval;

    	// Declare file descriptors for pipes
    	int fd_x_world, fd_z_world, fd_final;

	// Declare names for pipes
	char * myfifo_x_world = "/tmp/myfifo_x_world";
	char * myfifo_z_world = "/tmp/myfifo_z_world";
	char * myfifo_final = "/tmp/myfifo_final";
	
	// Create pipes
	mkfifo(myfifo_x_world, 0666);
	mkfifo(myfifo_z_world, 0666);
	mkfifo(myfifo_final, 0666);
	
	// Declare strings to hold position data from pipes
    	char pos_x[3];
	char pos_z[3];
	char pos_final[20];

	// Declare variables to hold position data
    	int px;
	int pz;
	int c=0;
	double real_px, real_pz;

	
	// Main loop
    	while(1){
	
		//open pipe connected to motor x
		fd_x_world = open(myfifo_x_world, O_RDONLY | O_NONBLOCK);
		if(fd_x_world == -1){
			// Write error message to log file
			write(log_wr, "Error while opening the pipe connected to motor x.\n", 52);
			return -1;
		}
		//open pipe connected to motor z
		fd_z_world = open(myfifo_z_world, O_RDONLY | O_NONBLOCK);
		if(fd_z_world == -1){
			// Write error message to log file
			write(log_wr, "Error while opening the pipe connected to motor z.\n", 52);
			return -1;
		}

		// Clear file descriptor set and set file descriptors for pipes
		FD_ZERO(&rfds);
		FD_SET(fd_x_world, &rfds);
		FD_SET(fd_z_world, &rfds);
		
		// Set timeout for select function
		tv.tv_sec = 0;
		tv.tv_usec = 50000;
		
		// Find maximum file descriptor
		int max = -1;
		if(fd_x_world < fd_z_world){
			max = fd_z_world;
		}
		else
			max = fd_x_world;
		
		// Declare string to hold log message
		char msg[50];
		
		// Wait for data to be available on one of the pipes
		retval = select(max+1, &rfds, NULL, NULL, &tv);
		
		// Check return value of select function
		if (retval < 0){
			
			// Write error message to log file
			write(log_wr, "Select: ", 9);
			write(log_wr, strerror(errno), strlen(strerror(errno)));
			write(log_wr, "\n", 2);
		}
		else if (retval > 0) {
			
			// If data is available on both pipes
			if(FD_ISSET(fd_x_world, &rfds) && FD_ISSET(fd_z_world, &rfds)){
			
				// Randomly choose which pipe to read from
				if(rand() % 2){

					// Read from pipe connected to motor x
					if(read(fd_x_world, pos_x, 3) == -1){
						// Write error message to log file
						write(log_wr, "Error while reading from the pipe connected to motor x\n", 56);
						return -1;
					}
					
					// Convert string to integer and store in px variable
					px = atoi(pos_x);

					// Add error to position and store in real_px variable
					real_px = addError(px);
                	
                			// Write message to log file
					sprintf(msg, "Current real horizontal position: %f\n", real_px);
					write(log_wr, msg, strlen(msg));
				
				}
				else{

					// Read from pipe connected to motor z
					if(read(fd_z_world, pos_z, 3) == -1){
						// Write error message to log file
						write(log_wr, "Error while reading from the pipe connected to motor z\n", 56);
						return -1;
					}

					// Convert string to integer and store in pz variable
					pz = atoi(pos_z);

					// Add error to position and store in real_pz variable
					real_pz = addError(pz);
                	
                			// Write message to log file
					sprintf(msg, "Current real vertical position: %f\n", real_pz);
					write(log_wr, msg, strlen(msg));

				}
			
			}
			
			// If data is only available on pipe connected to motor x
			else if(FD_ISSET(fd_x_world, &rfds)){

				// Read from pipe connected to motor x
				if(read(fd_x_world, pos_x, 3) == -1){
					
					// Write error message to log file
					write(log_wr, "Error while reading from the pipe connected to motor x\n", 56);
					return -1;
				}
				
				// Convert string to integer and store in px variable
				px = atoi(pos_x);

				// Add error to position and store in real_px variable
				real_px = addError(px);
				
				// Write message to log file
				sprintf(msg, "Current real horizontal position: %f\n", real_px);
				write(log_wr, msg, strlen(msg));

			}
			
			// If data is only available on pipe connected to motor z
			else if(FD_ISSET(fd_z_world, &rfds)){

				// Read from pipe connected to motor z
				if(read(fd_z_world, pos_z, 3) == -1){
					// Write error message to log file
					write(log_wr, "Error while reading from the pipe connected to motor z\n", 56);
					return -1;
				}

				// Convert string to integer and store in pz variable
				pz = atoi(pos_z);

				// Add error to position and store in real_pz variable
				real_pz = addError(pz);
				
				// Write message to log file
				sprintf(msg, "Current real vertical position: %f\n", real_pz);
				write(log_wr, msg, strlen(msg));

			}
			
		}
		
		//open pipe connected to inspection console
		fd_final = open(myfifo_final, O_WRONLY);
		if(fd_final == -1){
			
			// Write error message to log file
			write(log_wr, "Error while opening the pipe connected to inspection console.\n", 63);
			return -1;
		}

		// Write the real positions in the pipe connected to inspection console 
		sprintf(pos_final, "%f,%f", real_px, real_pz);
		if(write(fd_final, pos_final, strlen(pos_final)+1) == -1){
			
			// An error message is written to the log
			write(log_wr,"Error while writing in the pipe connected to inspection console.\n", 66);
			return -1;
		}
		
		/* The close function is used to close the fd_final file descriptor. 
   		If an error occurs, the function returns -1. */
		if(close(fd_final) == -1){
			
			// An error message is written to the log
			write(log_wr,"Error while closing the pipe connected to inspection console.\n", 63);
			return -1;
		}
		
		/* The close function is used to close the fd_x_world file descriptor. 
   		If an error occurs, the function returns -1. */
		if(close(fd_x_world) == -1){
			
			// An error message is written to the log
			write(log_wr,"Error while closing the pipe connected to motor x.\n", 52);
			return -1;
		}
		
		/* The close function is used to close the fd_z_world file descriptor. 
   		If an error occurs, the function returns -1. */
		if(close(fd_z_world) == -1){
			
			// An error message is written to the log
			write(log_wr,"Error while closing the pipe connected to motor z.\n", 52);
			return -1;
		}
		
	}

	/* The close function is used to close the log_wr file descriptor. */
	close(log_wr);
	
	/* The main function returns 0 to indicate that it has completed successfully. */
	return 0;

}
