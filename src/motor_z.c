#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(){

	int fd_z;
	
	int vz = 0;
	
	char * myfifo_z = "/tmp/myfifo_z";
	mkfifo(myfifo_z, 0666);
	
	char str_z[2];
	
	
	char incr_str[] = "1";
    	char decr_str[] = "2";
    	char stop_str[] = "0";
	
	//perror("Open fail");
	while(1){
	
		//open pipe connected to command console
		fd_z = open(myfifo_z, O_RDONLY);
		
		read(fd_z, str_z, strlen(str_z)+1);
		
		if(strcmp(str_z, incr_str) == 0){
			vz++;
			printf("Vz = %d\n", vz);
			fflush(stdout);
		}else if(strcmp(str_z, decr_str) == 0){
			vz--;
			printf("Vz = %d\n", vz);
			fflush(stdout);
		}else if(strcmp(str_z, stop_str) == 0){
			vz = 0;
			printf("Vz = %d\n", vz);
			fflush(stdout);
		}
		
		close(fd_z);
	
	}
	
	
}
