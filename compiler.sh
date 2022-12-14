#Create bin directory
mkdir -p bin &

#Create logFiles directory
mkdir -p logFiles &

#Compile the inspection console process
gcc src/inspection_console.c -lncurses -lm -o bin/inspection &

#Compile the command console process
gcc src/command_console.c -lncurses -o bin/command &

#Compile the master process
gcc src/master.c -o bin/master &

#Compile the motor x process
gcc src/motor_x.c -o bin/motor_x &

#Compile the motor z process
gcc src/motor_z.c -o bin/motor_z &

#Compile the world process
gcc src/world.c -o bin/world
