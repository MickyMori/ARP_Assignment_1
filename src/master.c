#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}

int main() {

  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };
  char * arg_list_motor_x[] = { "/usr/bin/konsole", "-e", "./bin/motor_x", NULL };
  char * arg_list_motor_z[] = { "/usr/bin/konsole", "-e", "./bin/motor_z", NULL };

  pid_t pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_t pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);
  pid_t pid_mx = spawn("/usr/bin/konsole", arg_list_motor_x);
  pid_t pid_mz = spawn("/usr/bin/konsole", arg_list_motor_z);

  int status;
  waitpid(pid_cmd, &status, 0);
  waitpid(pid_insp, &status, 0);
  waitpid(pid_mx, &status, 0);
  waitpid(pid_mz, &status, 0);
  
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

