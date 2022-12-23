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

// Declare a global variable for the velocity of motor x
int vx = 0;

// Declare file descriptors for the pipes used to communicate with command console and world
int fd_x, fd_x_world;

// Declare a file descriptor for the log file of motor x
int log_mx;
	
// Declare a global variable for the position of motor x
int pos_x = 0;

// Declare a file descriptor set and a timeval struct for use with the select function
fd_set rfds;
struct timeval tv;

// Declare a variable to store the return value of the select function
int retval;

// Declare a string to store the pathname of the pipes
char * myfifo_x = "/tmp/myfifo_x";
char * myfifo_x_world = "/tmp/myfifo_x_world";

// Declare a flag to keep track of whether the motor has received a stop signal
int stopped = 0;

// Declare two strings to store data received from the pipes
char str_x[2];
char str_pos_x[10];

// Declare a function to handle the stop signal
void handler_stop (int signal_number)
{
	
	// Write a message to the log file indicating that a stop signal was received
	write(log_mx, "Stop signal received.\n", 23);
	
	// Set the stopped flag to 1
	stopped = 1;
	
	// Set the velocity of motor x to 0
	vx = 0;
	
	// Write the updated velocity to the log file
	char msg[15];
	sprintf(msg, "Vx = %d\n", vx);
	write(log_mx, msg, strlen(msg));
	
} 

// Declare a function to handle the reset signal
void handler_reset (int signal_number)
{
	// Write a message to the log file indicating that a reset signal was received
	write(log_mx, "Reset signal received.\n", 24);

	// Create the pipe connected to world
	mkfifo(myfifo_x_world, 0666);
	
	// Set the velocity to move backwards
	vx = -1;
	
	// Keep moving the motor until it reaches the starting position
	while(pos_x > 0)
	{
		
		// If a stop signal has been received:
		if(stopped)
		{
			// Reset the stopped flag and return from the function
			stopped = 0;
			return;
		}
		
		// Update the position of motor x by adding the velocity
		pos_x += vx;

		// Convert the position to a string and store it in str_pos_x
		sprintf(str_pos_x,"%d", pos_x);

		// Open the second pipe for writing
		fd_x_world = open(myfifo_x_world, O_WRONLY);

		// If the pipe could not be opened:
		if(fd_x_world == -1){
			// Write an error message to the log file
			write(log_mx, "Error while opening the pipe connected to world.\n", 50);
		}

		// Write the position of motor x to the pipe connected to world
		if(write(fd_x_world, str_pos_x, strlen(str_pos_x)+1) == -1){
			// If an error occurs:
			// Write an error message to the log file
			write(log_mx, "Error while writing in the pipe connected to world.\n", 53);
		}

		// Close the pipe connected to world
		if(close(fd_x_world) == -1){
			
			// If an error occurs:
			// Write an error message to the log file
			write(log_mx,"Error while closing the pipe connected to world.\n", 50);
		}

		// Initialize the file descriptor set for the select function
		FD_ZERO(&rfds);
		FD_SET(fd_x, &rfds);
		
		// Set the timeout for the select function
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		// Wait for data to be available on the first pipe, or for the timeout to expire
		retval = select(fd_x+1, &rfds, NULL, NULL, &tv);
		
		// Declare a string to store a message to be written to the log file
		char msg[15];

		// If an error occurred in the select function:
		if (retval < 0){
			// Write an error message to the log file
			write(log_mx, "Select: ", 9);
			write(log_mx, strerror(errno), strlen(strerror(errno)));
			write(log_mx, "\n", 2);
		}
		// If data is available on the first pipe:
		else if (retval > 0) {
			
			// If the first pipe is ready for reading:
			if(FD_ISSET(fd_x, &rfds)){
				
				// Read data from the first pipe into str_x
				if(read(fd_x, str_x, 2) == -1){
					
					// If an error occurs:
					// Write an error message to the log file
					write(log_mx, "Error while reading from the pipe connected to command console\n", 64);
				}
	
			}
			
		}

	}


	// Set the velocity of motor x to 0
	vx = 0;

}

int main(){

	//open log file for motor x
	log_mx = open("logFiles/motor_x.log", O_WRONLY | O_APPEND | O_CREAT, 0666);  
	ftruncate(log_mx, 0);

	// create named pipe connected to command console
	mkfifo(myfifo_x, 0666);

	// create named pipe connected to world
	mkfifo(myfifo_x_world, 0666);
	
	// set up handler function for stop signal
	if(signal(SIGUSR1, handler_stop) == SIG_ERR){
		write(log_mx, "Error while receiving stop signal.\n", 36);
		return -1;
	}
	
	// set up handler function for reset signal
	if(signal(SIGUSR2, handler_reset) == SIG_ERR){
		write(log_mx, "Error while receiving reset signal.\n", 37);
		return -1;
	}
	
	// infinite loop
	while(1){
	
		//open pipe connected to command console
		fd_x = open(myfifo_x, O_RDONLY | O_NONBLOCK);
		if(fd_x == -1){
			write(log_mx, "Error while opening the pipe in connection to command console\n", 63);
			return -1;
		}
		
		// use select() to check if there is any data to be read from the pipe
		FD_ZERO(&rfds);
		FD_SET(fd_x, &rfds);
		
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		retval = select(fd_x+1, &rfds, NULL, NULL, &tv);
		
		char msg[15];

		// check if select() returned an error
		if (retval < 0){
			write(log_mx, "Select: ", 9);
			write(log_mx, strerror(errno), strlen(strerror(errno)));
			write(log_mx, "\n", 2);
		}
		
		// check if there is data to be read from the pipe
		else if (retval > 0) {
			if(FD_ISSET(fd_x, &rfds)){
				
				// read data from the pipe
				if(read(fd_x, str_x, 2) == -1){
					write(log_mx, "Error while reading from the pipe connected to command console\n", 64);
					return -1;
				}
	
				// update velocity based on data read from the pipe
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


		// update position if velocity is not zero
		if(vx != 0){
			
			pos_x += vx;

			// check if position is within the allowed range
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

			// convert position to string and write it to the named pipe connected to world
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
