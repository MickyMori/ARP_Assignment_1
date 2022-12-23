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

int log_ma; // File descriptor for log_ma

int status; // Status of a process

pid_t pid_cmd; // Process ID for command process
pid_t pid_mx; // Process ID for motor_x process
pid_t pid_mz; // Process ID for motor_z process
pid_t pid_wr; // Process ID for world process
pid_t pid_insp; // Process ID for inspection process

char msg[80]; // Buffer for messages

int flags[5] = {0,0,0,0,0}; // Flags for indicating whether a log file has been modified in the last 60 seconds

// Set the number of log files to monitor
const int num_log_files = sizeof(log_files) / sizeof(log_files[0]);

// Set the interval at which to check the log files (in seconds)
const int check_interval = 60;

/*
This function creates a child process by calling fork().
If the child process is successfully created, it calls execvp() to execute the specified program with the given argument list.
If there is an error while creating the child process or executing the program, an error message is written to log_ma.
Returns the child process ID on success, and 1 on failure.
*/
int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork(); // Create a child process

  if(child_pid < 0) { // Error while creating child process
    write(log_ma, "Error while forking.\n", 22); // Write error message to log_ma
    return 1;
  }

  else if(child_pid != 0) { // This is the parent process
    return child_pid; // Return child process ID
  }

  else { // This is the child process
    if(execvp (program, arg_list) == 0); // Execute the specified program with the given argument list
      write(log_ma, "Exec failed.\n", 14); // Write error message to log_ma
    return 1;
  }
}


/*
This function monitors the log files for activity.
It checks if the log files have been modified in the last 60 seconds.
If any of the log files have not been modified in the last 60 seconds, it writes a message to log_ma indicating that the log file has notbeen updated. If all of the log files have not been modified in the last 60 seconds, it writes a message to log_ma indicating that all of the log files have not been updated and kills all processes.
The function also checks if any of the processes have encountered an error or have terminated, in which case it writes a message to log_ma and kills all processes.
The function runs in an infinite loop, sleeping for 1 second at the end of each iteration.
*/
void watchdog(){

  sleep(1); // Sleep for 1 second

  while (1) {

    // Loop through the log files to monitor
    for (int i = 0; i < num_log_files; i++) {

      // Get the last modification time of the log file
      char cmd[1024];
      sprintf(cmd, "stat -c %Y %s", log_files[i]); // Get the last modification time of the log file in seconds since the epoch
      FILE *f = popen(cmd, "r"); // Open a pipe to execute the command
      time_t log_mtime = 0;
      fscanf(f, "%ld", &log_mtime); // Read the last modification time from the pipe
      pclose(f); // Close the pipe

      // Check if the log file has been modified in the last 60 seconds
      if (time(NULL) > log_mtime + check_interval) {

        // The log file has not been modified in the last 60 seconds
        if(flags[i] == 0){
          sprintf(msg, "%s has not been updated in 60 seconds.\n", log_files[i]); // Format the message
          write(log_ma, msg, strlen(msg)+1); // Write the message to log_ma
        }

        flags[i] = 1; // Set the flag for this log file
        
      }else{
        flags[i] = 0; // Reset the flag for this log file
      }

    }

    if(flags[0] && flags[1] && flags[2] && flags[3] && flags[4]){ // All log files have not been modified in the last 60 seconds

      write(log_ma, "All the processes have been inactive for the past 60 seconds. Killing all processes.\n", 86); // Write message to log_ma
      return; // Exit the function

    }

    if(waitpid(pid_cmd, &status, WNOHANG) == -1 || waitpid(pid_mx, &status, WNOHANG) == -1 || waitpid(pid_mz, &status, WNOHANG) == -1 || waitpid(pid_wr, &status, WNOHANG) == -1 || waitpid(pid_insp, &status, WNOHANG) == -1){ // An error has occurred or one of the processes has terminated
      write(log_ma, "An error occured inside one of the processes. Killing all processes.\n", 70); // Write message to log_ma
      return; // Exit the function
    }

  }

}


/*
This is the main function of the program.
It opens a log file named "master.log" in the "logFiles" directory for writing and appending, and truncates it if it already exists.
It then creates five child processes using the spawn() function, each to execute a different program:
- The first child process runs the "command" program in a new Konsole window.
- The second child process runs the "motor_x" program.
- The third child process runs the "motor_z" program.
- The fourth child process runs the "world" program.
- The fifth child process runs the "inspection" program in a new Konsole window.
The main function then calls the watchdog() function to monitor the log files and the child processes.
If the watchdog() function returns, the main function kills all of the child processes and waits for them to terminate before exiting.
It then prints the status of the main program and closes the log file before returning 0.
*/
int main() {

  log_ma = open("logFiles/master.log", O_WRONLY | O_APPEND | O_CREAT, 0666); // Open log file for writing and appending, create it if it doesn't exist, and set permissions to 0666
	ftruncate(log_ma, 0); // Truncate the log file

  // Set up argument lists for each of the programs to be run by the child processes
  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_motor_x[] = { "./bin/motor_x", NULL };
  char * arg_list_motor_z[] = { "./bin/motor_z", NULL };
  char * arg_list_world[] = { "./bin/world", NULL };
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };

  // Create the child processes
  pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_mx = spawn("./bin/motor_x", arg_list_motor_x);
  pid_mz = spawn("./bin/motor_z", arg_list_motor_z);
  pid_wr = spawn("./bin/world", arg_list_world);
  pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);

  watchdog(); // Monitor the log files and child processes

  // Kill all child processes
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

