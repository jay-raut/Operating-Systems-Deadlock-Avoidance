/**
 * Full Name: Jay Raut
 * Course ID: Section B
 * Description: Definition of a process
 */

#ifndef PROCESS_H
#define PROCESS_H
// representation of a a process
typedef struct Process {
    read_request *all_requests;
    int request_size;
    int current_request_pointer;
    int is_terminated;
    int was_aborted;
    int is_waiting;
    int *held_resource;
    int time_taken;
    int time_waiting;
    int pid;
    int compute_cycles;
    int currently_computing;
} Process;

#endif
