# Simulate Simple Resource Allocation for Deadlock Avoidance (FIFO and Banker)

## Usage 
Compile the program by using the make file provided and run using ./banker 'input-file'

## Output

The program will output the statistics for the FIFO and Bankers algorithm. The time taken, waiting time and percentage of time spent waiting for resources will be displayed. In FIFO, it is possible for a process(s) to be deadlocked. In this case 
the process is aborted. In bankers algorithm, deadlock is not possible. 

## Input format

All activites have a string followed by three non-negative integers. The first line in the file specifies the number of processes, resources types and number of units for that resource respectively. Example inputs are included in the repo. Example for
activites for a process is shown. 
```
initiate process-number resource-type initial-claim

request process-number resource-type number-requested 
release process-number resource-type number-released 

compute task-number number-of-cycles unused

terminate task-number unused unused
```
