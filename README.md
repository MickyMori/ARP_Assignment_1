# Advanced and Robot Programming - Assignment 1
## Authors: Rocca Giovanni, Moriconi Michele

<br>

### Introduction
An overview of this program function.<br>
[Go to Introduction](#introduction)

### Installation and Execution
How install and run this program in Linux.<br>
[Go to Installation and Execution](#installation_and_execution)

### Legend of Buttons
Legend of the buttons present in the two windows.<br>
[Go to Legend of Buttons](#legend_of_buttons)

### How it works
A rapid description of how the program works.<br>
[Go to How it works](#how_it_works)


<a name="introduction"></a>
### Introduction

This goal of this assignment is to test and deploy an interactive simulator of hoist with 2 degrees of freedom, in which two different consoles allow the user to activate the hoist.<br>
In the octagonal box there are two motors mx and mz, which displace the hoist along the two
respective axes. (Motions along axes have their bounds, from 0 to max_x along the x axis and from 0 to max_z along the z axis).<br>
From the user side there are two consoles (konsole windows), which contain the GUIs, one of these is the Inspection Console, that has a red button STOP (which purpose is to immediately stop all the movements along x and z axes) and an orange button RESET (which purpose is to reset all, dragging the hoist to the initial position), while the other one is the Command Console which contains the keys, clickable by the user, with different aims. 
This two consoles allow to simulate a real system. <br>

Our simulator is composed by 5 processes:  

<a name="legend_of_buttons"></a>
### Legend of Buttons

Command Console:

* Vx++ -> X axis increase
* Vx-- -> X axis decrease
* STP (between Vx-- and Vx++) -> X axis stop
* Vz++ -> Z axis increase
* Vz-- -> Z axis decrease
* STP (between Vz-- and Vz++) -> Z axis stop

Inspection Console:

* S (red button): emergency stop, the joist stops immediately until a command from the first console arrives
* R (orange button): reset, the joint stops, both axes go to a zero position and wait for commands (NB: it can be preempted by the S button)

<a name="installation_and_execution"></a>
### Installation and Execution

1.Open the terminal

2.Download the repository:

<pre><code>git clone https://github.com/MickyMori/ARP_Assignment_1</code></pre>

This repository contains:
- This 'README.md' file.
- The `src` folder, which contains the source code for the Command console, Inspection console, Motor_x, Motor_z, World and Master processes.
- The `include` folder contains all the data structures and methods used within the ncurses framework to build the two GUIs. Unless you want to expand the graphical capabilities of the UIs (which requires understanding how ncurses works), you can ignore the content of this folder, as it already provides you with all the necessary functionalities.
- The `compiler.sh` and `run.sh` files.

3.Compile the source files:

<pre><code>bash compiler.sh</code></pre>

Once this command has been run, `logFiles` and `bin` folders will be created.

4.Execute the program:

<pre><code>bash run.sh</code></pre>

If you want to read a specific log file of a process:

Go in the log files directory:
<pre><code>cd logFiles</code></pre>

Command console log file:
<pre><code>cat command.log</code></pre>

Inspection console log file:
<pre><code>cat inspection.log</code></pre>

Master log file:
<pre><code>cat master.log</code></pre>

Motor x log file:
<pre><code>cat motor_x.log</code></pre>

Motor z log file:
<pre><code>cat motor_z.log</code></pre>

World log file:
<pre><code>cat world.log</code></pre>

<a name="how_it_works"></a>
### How it works

There are 5 processes:

* <b>Command Console</b>: reads the commands clicked in the window from the user; 
To see these commands [click here](#legend_of_buttons).

* <b>Inspection Console</b>: receives from motor x and motor z the hoist position and show it on the specific interface, it also contains the `stop` button and the `reset` button;
To see these buttons [click here](#legend_of_buttons).

* <b>Motor X</b>: receives commands, simulates the motion along x axis and sends the real time position to the World process.

* <b>Motor Z</b>: receives commands, simulates the motion along z axis and sends the real time position to the World process.

* <b>World</b>: add the errors to the positions received by Motor X and Motor Z and sends this recalculated position to the Inspection Console.

* <b>Master</b>: his main function is to start all the processes and it contains a `Watchdog` function that checks all the processes periodically and kills all of them in
    case they are all idle.

