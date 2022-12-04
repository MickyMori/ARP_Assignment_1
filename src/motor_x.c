#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(){

	int fd_x;
	
	int vx = 0;
	
	char * myfifo_x = "/tmp/myfifo_x";
	mkfifo(myfifo_x, 0666);
	
	char str_x[1];
	
	
	
	while(1){
	
		//open pipe connected to command console
		fd_x = open(myfifo_x, O_RDONLY);
		
		read(fd_x, str_x, strlen(str_x)+1);
		
	
		if(strcmp(str_x, "1") == 0){
			++vx;
			printf("Vx = %d\n", vx);
			fflush(stdout);
		}else if(strcmp(str_x, "2") == 0){
			vx--;
			printf("Vx = %d\n", vx);
			fflush(stdout);
		}else if(strcmp(str_x, "0") == 0){
			vx = 0;
			printf("Vx = %d\n", vx);
			fflush(stdout);
		}
		
		close(fd_x);
		
	}
	
	
	
}
