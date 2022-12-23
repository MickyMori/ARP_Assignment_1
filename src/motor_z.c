#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

// Global variable to store the velocity of the motor
int vz = 0;

// File descriptors for the pipes
int fd_z, fd_z_world;

// File descriptor for the log file
int log_mz;

// File descriptor set and timeval structure for the select function
fd_set rfds;
struct timeval tv;
int retval;

// Paths for the two pipes
char * myfifo_z = "/tmp/myfifo_z";
char * myfifo_z_world = "/tmp/myfifo_z_world";

// Buffer for reading from the pipe connected to the command console
char str_z[2];

// Buffer for writing the position to the pipe connected to the world
char str_pos_z[10];

// Global variable to store the current position of the motor
int pos_z = 0;

// Flag to store whether the motor has been stopped or not
int stopped;

// Signal handler function for the stop signal
void handler_stop(int signal_number)
{
	// Write a message to the log file
	write(log_mz, "Stop signal received.\n", 23);
	
	// Set the stopped flag and reset the velocity
	stopped = 1;
	vz = 0;
	
	// Write the current velocity to the log file
	char msg[15];
	sprintf(msg, "Vz = %d\n", vz);
	write(log_mz, msg, strlen(msg));
}

// Signal handler function for the reset signal
void handler_reset (int signal_number)
{
	// Write a message to the log file
	write(log_mz, "Reset signal received.\n", 24);

	// Create the pipe connected to the world if it doesn't already exist
	mkfifo(myfifo_z_world, 0666);
	
	// Set the velocity to move backwards
	vz = -1;
	
	// Keep moving the motor until it reaches the starting position
	while(pos_z != 0)
	{
		// Check if the stop signal has been received
		if(stopped)
		{
			// Reset the stopped flag and return from the function
			stopped = 0;
			return;
		}

		// Update the current position of the motor
		pos_z += vz;

		// Convert the position to a string and write it to the pipe connected to the world
		sprintf(str_pos_z,"%d", pos_z);

		fd_z_world = open(myfifo_z_world, O_WRONLY);

		// Check if there was an error while opening the pipe connected to the world
		if(fd_z_world == -1){
			write(log_mz, "Error while opening the pipe connected to world.\n", 50);
		}

		// Write the current position to the pipe connected to the world
		if(write(fd_z_world, str_pos_z, strlen(str_pos_z)+1) == -1){
			write(log_mz, "Error while writing in the pipe connected to world.\n", 53);
		}

		// Close the pipe connected to the world
		if(close(fd_z_world) == -1){
			write(log_mz,"Error while closing the pipe connected to world.\n", 50);
		}

		// Clear the file descriptor set and set the file descriptor for the pipe connected to the command console
		FD_ZERO(&rfds);
		FD_SET(fd_z, &rfds);
		
		// Set the timeout for the select function
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		// Wait for input on the pipe connected to the command console
		retval = select(fd_z+1, &rfds, NULL, NULL, &tv);

		char msg[15];

		// Check if there was an error or if input was received on the pipe
		if (retval < 0){
			
			// Write an error message to the log file
			write(log_mz, "Select: ", 9);
			write(log_mz, strerror(errno), strlen(strerror(errno)));
			write(log_mz, "\n", 2);
		}
		else if (retval > 0) {
			
			// Check if the file descriptor for the pipe connected to the command console is set
			if(FD_ISSET(fd_z, &rfds)){
		
				// Read from the pipe connected to the command console
				if(read(fd_z, str_z, 2) == -1){
					write(log_mz, "Error while reading from the pipe connected to command console\n", 64);
				}

			}
		}
	}
	// Set the velocity of motor z to 0
	vz = 0;

}

int main(){

	//open log file for motor z in append mode
	log_mz = open("logFiles/motor_z.log", O_WRONLY | O_APPEND | O_CREAT, 0666);  
	
	// Truncate the log file to zero size
	ftruncate(log_mz, 0);
	
	// Create the pipe connected to the command console if it doesn't already exist
	mkfifo(myfifo_z, 0666);

	// Create the pipe connected to the world if it doesn't already exist
	mkfifo(myfifo_z_world, 0666);

	// Set the signal handlers for the stop and reset signals
	if(signal(SIGUSR1, handler_stop) == SIG_ERR){
		write(log_mz, "Error while receiving stop signal.\n", 36);
		return -1;
	}
	
	if(signal(SIGUSR2, handler_reset) == SIG_ERR){
		write(log_mz, "Error while receiving reset signal.\n", 37);
		return -1;
	}
	
	// Endless loop to continuously check for input on the pipe connected to the command console
	while(1){
		
		// Open the pipe connected to the command console
		fd_z = open(myfifo_z, O_RDONLY | O_NONBLOCK);
		if(fd_z == -1){
			write(log_mz, "Error while opening the pipe in connection to command console\n", 63);
			return -1;
		}
		
		// Set the file descriptor set and timeout for the select function
		FD_ZERO(&rfds);
		FD_SET(fd_z, &rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		// Wait for input on the pipe connected to the command console
		retval = select(fd_z+1, &rfds, NULL, NULL, &tv);

		char msg[15];

		if (retval < 0){
			// Write an error message to the log file
			write(log_mz, "Select: ", 9);
			write(log_mz, strerror(errno), strlen(strerror(errno)));
			write(log_mz, "\n", 2);
		}
		else if (retval > 0) {
			
			// Check if the file descriptor for the pipe connected to the command console is set
			if(FD_ISSET(fd_z, &rfds)){
		
				// Read from the pipe connected to the command console
				if(read(fd_z, str_z, 2) == -1){
					write(log_mz, "Error while reading from the pipe connected to command console\n", 64);
					return -1;
				}

				// Write the current velocity to the log file
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

		// Update the position of the motor
		if(vz != 0){
			pos_z += vz;
			//check if maximum position has been reached
			if(pos_z > 9){
				write(log_mz, "Maximum vertical position reached... the program is setting vertical velocity to zero.\n", 88);
				vz = 0;
				pos_z = 10;
			}
			//check if minimum position has been reached
			if(pos_z < 0){
				write(log_mz, "Minimum vertical position reached... the program is setting vertical velocity to zero.\n", 88);
				vz = 0;
				pos_z = 0;
			}

			// Convert the position to a string and write it to the pipe connected to the world
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

	// Close the log file and the pipe connected to the command console
	close(log_mz);

	return 0;
}
