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


// Set the names of the log files to monitor
const char *log_files[] = { "logFiles/command.log", "logFiles/inspection.log", "logFiles/world.log", "logFiles/motor_x.log", "logFiles/motor_z.log" };

int log_ma;

int status;

pid_t pid_cmd;
pid_t pid_mx;
pid_t pid_mz;
pid_t pid_wr;
pid_t pid_insp;

char msg[80];

int flags[5] = {0,0,0,0,0};

// Set the number of log files to monitor
const int num_log_files = sizeof(log_files) / sizeof(log_files[0]);

// Set the interval at which to check the log files (in seconds)
const int check_interval = 60;

int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    write(log_ma, "Error while forking.\n", 22);
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
      write(log_ma, "Exec failed.\n", 14);
    return 1;
  }
}

void watchdog(){

  sleep(1);

  while (1) {

    // Loop through the log files to monitor
    for (int i = 0; i < num_log_files; i++) {

      // Get the last modification time of the log file
      char cmd[1024];
      sprintf(cmd, "stat -c %Y %s", log_files[i]);
      FILE *f = popen(cmd, "r");
      time_t log_mtime = 0;
      fscanf(f, "%ld", &log_mtime);
      pclose(f);

      // Check if the log file has been modified in the last 60 seconds
      if (time(NULL) > log_mtime + check_interval) {

        // The log file has not been modified in the last 60 seconds
        if(flags[i] == 0){
          sprintf(msg, "%s has not been updated in 60 seconds.\n", log_files[i]);
          write(log_ma, msg, strlen(msg)+1);
        }

        flags[i] = 1;
        
      }else{
        flags[i] = 0;
      }

    }

    if(flags[0] && flags[1] && flags[2] && flags[3] && flags[4]){

      write(log_ma, "All the processes have been inactive for the past 60 seconds. Killing all processes.\n", 86);
      return;

    }

    if(waitpid(pid_cmd, &status, WNOHANG) == -1 || waitpid(pid_mx, &status, WNOHANG) == -1 || waitpid(pid_mz, &status, WNOHANG) == -1 || waitpid(pid_wr, &status, WNOHANG) == -1 || waitpid(pid_insp, &status, WNOHANG) == -1){
      write(log_ma, "An error occured inside one of the processes. Killing all processes.\n", 70);
      return;
    }

  }

}

int main() {

  log_ma = open("logFiles/master.log", O_WRONLY | O_APPEND | O_CREAT, 0666);  
	ftruncate(log_ma, 0);

  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_motor_x[] = { "./bin/motor_x", NULL };
  char * arg_list_motor_z[] = { "./bin/motor_z", NULL };
  char * arg_list_world[] = { "./bin/world", NULL };
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };

  pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_mx = spawn("./bin/motor_x", arg_list_motor_x);
  pid_mz = spawn("./bin/motor_z", arg_list_motor_z);
  pid_wr = spawn("./bin/world", arg_list_world);
  pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);

  watchdog();

  kill(pid_cmd, SIGKILL);
  kill(pid_mx, SIGKILL);
  kill(pid_mz, SIGKILL);
  kill(pid_wr, SIGKILL);
  kill(pid_insp, SIGKILL);

  
  //waitpid(pid_cmd, &status, 0);
  //waitpid(pid_mx, &status, 0);
  //waitpid(pid_mz, &status, 0);
  //waitpid(pid_wr, &status, 0);
  //waitpid(pid_insp, &status, 0);
  
  printf ("Main program exiting with status %d\n", status);
  close(log_ma);
  return 0;
}

