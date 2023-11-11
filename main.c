#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check_request_type.c"
#include "math.h"
#include "process.h"
#include "resource.h"

#define MAX_VALUE 0x7FFFFFFF
#define MAX_LINE_LENGTH 100
#define MAX_INITIAL_UNITS 100
#define MAX_TASK_REQUESTS 50
#define MAX_REQUEST_SIZE 50
void FIFO(resource *, Process[]);
void Banker(resource *, Process[]);
void print_units(resource *);
int check_all_terminated(resource *, Process[]);
Process *initalize_FIFO(char **);
Process *initalize_Banker(char **);
int number_of_processes = 0;
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Missing Arguments\n");
        return 0;
    }
    Process *processes_FIFO = initalize_FIFO(argv);
    Process *processes_Banker = initalize_Banker(argv);
    if (processes_FIFO != NULL) {
        int total_waiting_time = 0;
        int total_finishing_time = 0;
        int total_waiting_time_Banker = 0;
        int total_finishing_time_Banker = 0;
        int total_percentage = 0;
        printf("%23s %30s\n", "FIFO", "BANKER'S");
        for (int i = 0; i < number_of_processes; i++) {
            if (processes_FIFO[i].was_aborted == 1) {
                printf("%10s %-7d %-11s", "Task", i + 1, "aborted");
            } else {
                int finish_time = processes_FIFO[i].time_taken;
                int waiting_time = processes_FIFO[i].time_waiting;
                int percentage_waiting = (int)round((((double)waiting_time / (double)finish_time) * 100));
                total_finishing_time += finish_time;
                total_waiting_time += waiting_time;
                printf("%10s %d %7d %3d %4d%%", "Task", i + 1, finish_time, waiting_time, percentage_waiting);
            }
            if (processes_Banker[i].was_aborted == 1) {
                printf("%10s %-7d %s\n", "Task", i + 1, "aborted");
            } else {
                int finish_time = processes_Banker[i].time_taken;
                int waiting_time = processes_Banker[i].time_waiting;
                int percentage_waiting = (int)round((((double)waiting_time / (double)finish_time) * 100));
                total_finishing_time_Banker += finish_time;
                total_waiting_time_Banker += waiting_time;
                printf("%10s %d %7d %3d %4d%%\n", "Task", i + 1, finish_time, waiting_time, percentage_waiting);
            }
        }
        total_percentage = (int)(round(((double)total_waiting_time / (double)total_finishing_time) * 100));
        printf("%11s %8d %3d %4d%%", "total", total_finishing_time, total_waiting_time, total_percentage);
        total_percentage = (int)(round(((double)total_waiting_time_Banker / (double)total_finishing_time_Banker) * 100));
        printf("%11s %8d %3d %4d%%\n", "total", total_finishing_time_Banker, total_waiting_time_Banker, total_percentage);
    }
    free(processes_FIFO);
    free(processes_Banker);
    return 0;
}

void FIFO(resource *resource_count, Process processes[]) {
    int time_taken = 0;
    Process *request_queue[resource_count->process_count];  // put all requests in the queue
    Process *blocked_queue[resource_count->process_count];  // blocked queue for blocked tasks
    Process *release_queue[resource_count->process_count];
    int *released_resource = calloc(resource_count->resource_types, sizeof(int));
    int request_queue_pointer = -1;
    int release_queue_pointer = -1;
    int blocked_queue_pointer = -1;
    int pending_release = 0;
    while (check_all_terminated(resource_count, processes) == 0) {
        if (pending_release == 1) {
            for (int i = 0; i < resource_count->resource_types; i++) {  // releasing resources into resource_count
                resource_count->resource_available[i] += released_resource[i];
                released_resource[i] = 0;
            }
            pending_release = 0;
        }

        for (int i = 0; i < resource_count->process_count; i++) {
            if (processes[i].is_terminated == 1 || processes[i].was_aborted == 1 || processes[i].is_waiting == 1) {  // skip the process
                continue;
            }
            int get_current_pointer = processes[i].current_request_pointer;
            read_request *current_request = &processes[i].all_requests[get_current_pointer];
            if (check_initiate(current_request->request_type) == 1) {
                // do nothing for initiate
            } else if (check_request(current_request->request_type) == 1) {  // if process is requesting then place into request queue
                request_queue_pointer++;
                request_queue[request_queue_pointer] = &processes[i];
            } else if (check_release(current_request->request_type) == 1) {  // if process wants to release then place release queue
                release_queue_pointer++;
                release_queue[release_queue_pointer] = &processes[i];
            } else if (check_compute(current_request->request_type) == 1) {
                processes[i].currently_computing = 1;
                processes[i].compute_cycles++;
            } else if (check_terminated(current_request->request_type) == 1) {  // if the process has terminated, set terminate flag
                processes[i].is_terminated = 1;
                processes[i].time_taken = time_taken;
            }
        }
        for (int i = 0; i < resource_count->process_count; i++) {  // check if compute processes are done
            if (processes[i].currently_computing == 1) {
                int current_request = processes[i].current_request_pointer;
                int get_total_compute_cycles = processes[i].all_requests[current_request].resource_type;
                if (processes[i].compute_cycles >= get_total_compute_cycles) {  // done with compute cycles
                    processes[i].currently_computing = 0;
                    processes[i].compute_cycles = 0;
                }
            }
        }

        for (int i = 0; i <= release_queue_pointer; i++) {                    // releasing the resources
            int current_request = release_queue[i]->current_request_pointer;  // get current request to know how much to release
            int unit_type = release_queue[i]->all_requests[current_request].resource_type;
            int unit_amount = release_queue[i]->all_requests[current_request].resource_units;
            released_resource[unit_type - 1] += unit_amount;
            release_queue[i]->held_resource[unit_type - 1] -= unit_amount;
            pending_release = 1;
        }
        release_queue_pointer = -1;

        int current_successful_request = 0;                 // keep track of successful requests
        for (int i = 0; i <= blocked_queue_pointer; i++) {  // deal with blocked requests first
            int current_request = blocked_queue[i]->current_request_pointer;
            int unit_type = blocked_queue[i]->all_requests[current_request].resource_type;
            int unit_amount = blocked_queue[i]->all_requests[current_request].resource_units;
            if ((resource_count->resource_available[unit_type - 1] - unit_amount) < 0) {  // if request cannot be satisfied then stay at blocked state
                // do nothing
            } else {
                resource_count->resource_available[unit_type - 1] -= unit_amount;  // grant the resource
                blocked_queue[i]->held_resource[unit_type - 1] += unit_amount;
                blocked_queue[i]->is_waiting = 0;
                current_successful_request++;  // request is successful
                for (int j = i; j < blocked_queue_pointer; j++) {
                    blocked_queue[j] = blocked_queue[j + 1];
                }
                blocked_queue_pointer--;
                i--;
                continue;
            }
        }

        for (int i = 0; i <= request_queue_pointer; i++) {
            int current_request = request_queue[i]->current_request_pointer;  // get current request to know how much to request
            int unit_type = request_queue[i]->all_requests[current_request].resource_type;
            int unit_amount = request_queue[i]->all_requests[current_request].resource_units;
            if ((resource_count->resource_available[unit_type - 1] - unit_amount) < 0) {  // if request cannot be satisfied then move to blocked state
                blocked_queue_pointer++;
                request_queue[i]->is_waiting = 1;
                blocked_queue[blocked_queue_pointer] = request_queue[i];
                for (int j = i; j < request_queue_pointer; j++) {  // remove from request queue
                    request_queue[j] = request_queue[j + 1];
                }
                request_queue_pointer--;
                i--;
                continue;
            } else {  // the request is successful
                current_successful_request++;
                resource_count->resource_available[unit_type - 1] -= unit_amount;  // grant the resource
                request_queue[i]->held_resource[unit_type - 1] += unit_amount;
                for (int j = i; j < request_queue_pointer; j++) {  // remove from request queue
                    request_queue[j] = request_queue[j + 1];
                }
                request_queue_pointer--;
                i--;
                continue;
            }
        }
        if (current_successful_request == 0 && pending_release == 0 && (blocked_queue_pointer >= 0 || request_queue_pointer >= 0)) {  // all requests are blocked including blocked queue so start aborting processes
            while (1) {                                                                                                               // keep aborting processes until the deadlock is gone
                int pid = MAX_VALUE;
                int smallest_index;
                int from_blocked_or_request = -1;                   // 0 from blocked queue 1 from request queue
                for (int i = 0; i <= blocked_queue_pointer; i++) {  // we must find the smallest process pid from request and blocked queue
                    if (blocked_queue[i]->pid < pid) {
                        pid = blocked_queue[i]->pid;
                        from_blocked_or_request = 0;
                        smallest_index = i;
                    }
                }
                for (int i = 0; i <= request_queue_pointer; i++) {
                    if (request_queue[i]->pid < pid) {
                        pid = request_queue[i]->pid;
                        from_blocked_or_request = 1;
                        smallest_index = i;
                    }
                }
                if (from_blocked_or_request == 0) {  // smallest process is in blocked queue
                    pending_release = 1;
                    blocked_queue[smallest_index]->was_aborted = 1;
                    blocked_queue[smallest_index]->is_terminated = 1;
                    blocked_queue[smallest_index]->time_taken = time_taken;
                    for (int i = 0; i < resource_count->resource_types; i++) {
                        released_resource[i] += blocked_queue[smallest_index]->held_resource[i];
                        blocked_queue[smallest_index]->held_resource[i] = 0;
                    }
                    for (int i = smallest_index; i < blocked_queue_pointer; i++) {
                        blocked_queue[i] = blocked_queue[i + 1];
                    }
                    blocked_queue_pointer--;

                } else if (from_blocked_or_request == 1) {  // smallest process is in request queue
                    pending_release = 1;
                    request_queue[smallest_index]->was_aborted = 1;
                    request_queue[smallest_index]->is_terminated = 1;
                    request_queue[smallest_index]->time_taken = time_taken;
                    for (int i = 0; i < resource_count->resource_types; i++) {
                        released_resource[i] += request_queue[smallest_index]->held_resource[i];
                        request_queue[smallest_index]->held_resource[i] = 0;
                    }
                    for (int i = smallest_index; i < request_queue_pointer; i++) {
                        request_queue[i] = request_queue[i + 1];
                    }
                    request_queue_pointer--;
                }
                // checking if the deadlock still is there by counting the number of processes blocked
                int deadlock_count = 0;
                for (int i = 0; i <= release_queue_pointer; i++) {
                    int current_request_pointer = release_queue[i]->current_request_pointer;
                    int request_type = release_queue[i]->all_requests[current_request_pointer].resource_type;
                    int request_amount = release_queue[i]->all_requests[current_request_pointer].resource_units;
                    if ((resource_count->resource_available[request_type - 1] - request_amount + released_resource[request_type - 1]) < 0) {
                        deadlock_count++;
                    }
                }
                for (int i = 0; i <= blocked_queue_pointer; i++) {
                    int current_request_pointer = blocked_queue[i]->current_request_pointer;
                    int request_type = blocked_queue[i]->all_requests[current_request_pointer].resource_type;
                    int request_amount = blocked_queue[i]->all_requests[current_request_pointer].resource_units;
                    if ((resource_count->resource_available[request_type - 1] - request_amount + +released_resource[request_type - 1]) < 0) {
                        deadlock_count++;
                    }
                }
                if (deadlock_count != blocked_queue_pointer + 1 + request_queue_pointer + 1) {  // if the deadlock is not there then break, deadlock is checked by comparing
                                                                                                //  number of deadlocked processes to number of processes in blocked and request queue
                    break;
                }
            }
        }
        time_taken++;
        for (int i = 0; i < resource_count->process_count; i++) {  // increament processes
            if (processes[i].is_waiting == 0 && processes[i].is_terminated == 0 && processes[i].was_aborted == 0 && processes[i].currently_computing == 0) processes[i].current_request_pointer++;
            if (processes[i].is_waiting == 1 && processes[i].is_terminated == 0) {
                processes[i].time_waiting++;
            }
        }
    }
    free(released_resource);
}

void Banker(resource *resource_count, Process processes[]) {
    int time_taken = 0;
    Process *request_queue[resource_count->process_count];  // put all requests in the queue
    Process *blocked_queue[resource_count->process_count];  // blocked queue for blocked tasks
    Process *release_queue[resource_count->process_count];
    int *released_resource = calloc(resource_count->resource_types, sizeof(int));
    int request_queue_pointer = -1;
    int release_queue_pointer = -1;
    int blocked_queue_pointer = -1;
    int pending_release = 0;
    while (check_all_terminated(resource_count, processes) == 0) {
        if (pending_release == 1) {
            for (int i = 0; i < resource_count->resource_types; i++) {  // releasing resources into resource_count
                resource_count->resource_available[i] += released_resource[i];
                released_resource[i] = 0;
            }
            pending_release = 0;
        }
        for (int i = 0; i < resource_count->process_count; i++) {
            if (processes[i].is_terminated == 1 || processes[i].was_aborted == 1 || processes[i].is_waiting == 1) {  // skip the process
                continue;
            }
            int get_current_pointer = processes[i].current_request_pointer;
            read_request *current_request = &processes[i].all_requests[get_current_pointer];
            printf("%s %d %d %d\n", current_request->request_type, current_request->process_id, current_request->resource_type, current_request->resource_units);

            if (check_initiate(current_request->request_type) == 1) {
                int get_unit_type = current_request->resource_type;
                int get_unit_amount = current_request->resource_units;
                if (resource_count->resource_available[get_unit_type - 1] < get_unit_amount) {  // if the initial request is more than system avaliable then abort
                    processes[i].was_aborted = 1;
                    processes[i].is_terminated = 1;
                    continue;
                }
                // store initial claims into the processes
                processes[i].initial_claim[get_unit_type - 1] = get_unit_amount;
            } else if (check_compute(current_request->request_type) == 1) {
                processes[i].currently_computing = 1;
                processes[i].compute_cycles++;
            } else if (check_request(current_request->request_type) == 1) {
                request_queue_pointer++;
                request_queue[request_queue_pointer] = &processes[i];
            } else if (check_release(current_request->request_type) == 1) {
                release_queue_pointer++;
                release_queue[release_queue_pointer] = &processes[i];
            } else if (check_terminated(current_request->request_type) == 1) {
                processes[i].is_terminated = 1;
                processes[i].time_taken = time_taken;
            }
        }
        for (int i = 0; i < resource_count->process_count; i++) {  // check if compute processes are done
            if (processes[i].currently_computing == 1) {
                int current_request = processes[i].current_request_pointer;
                int get_total_compute_cycles = processes[i].all_requests[current_request].resource_type;
                if (processes[i].compute_cycles >= get_total_compute_cycles) {  // done with compute cycles
                    processes[i].currently_computing = 0;
                    processes[i].compute_cycles = 0;
                }
            }
        }

        for (int i = 0; i <= release_queue_pointer; i++) {                    // releasing the resources
            int current_request = release_queue[i]->current_request_pointer;  // get current request to know how much to release
            int unit_type = release_queue[i]->all_requests[current_request].resource_type;
            int unit_amount = release_queue[i]->all_requests[current_request].resource_units;
            released_resource[unit_type - 1] += unit_amount;
            release_queue[i]->held_resource[unit_type - 1] -= unit_amount;
            pending_release = 1;
        }
        release_queue_pointer = -1;

        for (int i = 0; i <= blocked_queue_pointer; i++) {
            int current_request = blocked_queue[i]->current_request_pointer;
            int unit_type = blocked_queue[i]->all_requests[current_request].resource_type;
            int unit_amount = blocked_queue[i]->all_requests[current_request].resource_units;
            int move_blocked = 0;
            for (int j = 0; j < resource_count->resource_types; j++) {  // check if process can run
                if (resource_count->resource_available[j] + blocked_queue[i]->held_resource[j] < blocked_queue[i]->initial_claim[j]) {
                    move_blocked = 1;
                    break;
                }
            }
            if (move_blocked == 1) {
                // do nothing
            } else {
                resource_count->resource_available[unit_type - 1] -= unit_amount;
                blocked_queue[i]->held_resource[unit_type - 1] += unit_amount;
                blocked_queue[i]->is_waiting = 0;

                for (int j = i; j < blocked_queue_pointer; j++) {
                    blocked_queue[j] = blocked_queue[j + 1];
                }
                blocked_queue_pointer--;
                i--;
                continue;
            }
        }

        for (int i = 0; i <= request_queue_pointer; i++) {
            int current_request = request_queue[i]->current_request_pointer;  // get current request to know how much to request
            int unit_type = request_queue[i]->all_requests[current_request].resource_type;
            int unit_amount = request_queue[i]->all_requests[current_request].resource_units;
            if ((request_queue[i]->held_resource[unit_type - 1] + unit_amount) > request_queue[i]->initial_claim[unit_type - 1]) {  // trying to exceed initial claim
                request_queue[i]->is_terminated = 1;
                request_queue[i]->was_aborted = 1;
                released_resource[unit_type - 1] += unit_amount;
                request_queue[i]->held_resource[unit_type - 1] -= unit_amount;
                pending_release = 1;
                for (int j = i; j < request_queue_pointer; j++) {
                    request_queue[j] = request_queue[j + 1];
                }
                request_queue_pointer--;
                i--;
                continue;
            }
            int move_blocked = 0;
            for (int j = 0; j < resource_count->resource_types; j++) {  // check if process can run
                if (resource_count->resource_available[j] + request_queue[i]->held_resource[j] < request_queue[i]->initial_claim[j]) {
                    move_blocked = 1;
                    break;
                }
            }
            if (move_blocked == 1) {
                blocked_queue_pointer++;
                blocked_queue[blocked_queue_pointer] = request_queue[i];
                request_queue[i]->is_waiting = 1;
                for (int j = i; j < request_queue_pointer; j++) {
                    request_queue[j] = request_queue[j + 1];
                }
                request_queue_pointer--;
                i--;
                continue;
            } else {
                resource_count->resource_available[unit_type - 1] -= unit_amount;  // grant the resource
                request_queue[i]->held_resource[unit_type - 1] += unit_amount;
                for (int j = i; j < request_queue_pointer; j++) {  // remove from request queue
                    request_queue[j] = request_queue[j + 1];
                }
                request_queue_pointer--;
                i--;
                continue;
            }
        }

        for (int i = 0; i < resource_count->process_count; i++) {  // increament processes
            if (processes[i].is_waiting == 0 && processes[i].is_terminated == 0 && processes[i].was_aborted == 0 && processes[i].currently_computing == 0) processes[i].current_request_pointer++;
            if (processes[i].is_waiting == 1 && processes[i].is_terminated == 0) {
                processes[i].time_waiting++;
            }
        }

        for (int i = 0; i < resource_count->process_count; i++) {
            for (int j = 0; j < resource_count->resource_types; j++) {
                printf("Process %d has %d at %d\n", i + 1, processes[i].held_resource[j], time_taken);
            }
        }
        time_taken++;
        printf("\n");
    }
    for (int i = 0; i < resource_count->resource_types; i++) {
        printf("Resource left %d\n", resource_count->resource_available[i]);
    }
    printf("Time Taken %d\n", time_taken);
    for (int i = 0; i < resource_count->process_count; i++) {
        printf("Process %d finished at %d\n", i + 1, processes[i].time_taken);
    }
    free(released_resource);
}

Process *initalize_FIFO(char **argv) {
    FILE *file_pointer;
    file_pointer = fopen(argv[1], "r");
    resource resource_count;

    char resource_count_string[MAX_LINE_LENGTH];  // read first line and get info
    fgets(resource_count_string, sizeof(resource_count), file_pointer);
    resource_count_string[sizeof(resource_count_string) - 1] = '\0';
    char *token;
    int number;
    int store_numbers[MAX_INITIAL_UNITS];
    int store_numbers_pointer = 0;
    token = strtok(resource_count_string, " ");
    while (token != NULL && token[0] != ' ') {
        // Convert the token to an integer using sscanf
        if (sscanf(token, "%d", &number) == 1) {
            // insert into store_numbers_array
            store_numbers[store_numbers_pointer] = number;
            store_numbers_pointer++;
        }
        // Get the next token
        token = strtok(NULL, " ");
    }  // this block is for reading the first line in the file and placing values into the store_numbers array
    if (store_numbers_pointer < 3) {
        printf("Not enough arguments in input\n");
        return NULL;
    }
    resource_count.process_count = store_numbers[0];   // place number of processes
    resource_count.resource_types = store_numbers[1];  // number of resource types
    int j = 2;
    int resource_types_array[resource_count.resource_types];  // placing the resource amounts for each type
    for (int i = 0; i < resource_count.resource_types; i++) {
        resource_types_array[i] = store_numbers[j];
        j++;
    }
    resource_count.resource_available = resource_types_array;                    // reference the array in resource count
    Process *processes = calloc(resource_count.process_count, sizeof(Process));  // all processes
    for (int i = 0; i < resource_count.process_count; i++) {                     // get all request for each process and store into memory
        read_request *requests = (read_request *)calloc(MAX_TASK_REQUESTS, sizeof(read_request));
        int current_request = 0;
        while (1) {  // keep reading until terminated
            char *request_type = calloc(MAX_REQUEST_SIZE, sizeof(char));
            fscanf(file_pointer, "%s %d %d %d", request_type, &requests[current_request].process_id, &requests[current_request].resource_type, &requests[current_request].resource_units);
            requests[current_request].request_type = request_type;
            processes[i].pid = requests[current_request].process_id;
            current_request++;
            if (check_terminated(request_type)) {
                break;
            }
        }
        processes[i].is_terminated = 0;
        processes[i].was_aborted = 0;
        processes[i].is_waiting = 0;
        processes[i].held_resource = calloc(resource_count.resource_types, sizeof(int));
        processes[i].time_taken = 0;
        processes[i].time_waiting = 0;
        processes[i].current_request_pointer = 0;
        processes[i].request_size = current_request;
        processes[i].all_requests = requests;
        processes[i].currently_computing = 0;
        processes[i].compute_cycles = 0;
        processes[i].initial_claim = calloc(resource_count.resource_types, sizeof(int));
    }

    number_of_processes = resource_count.process_count;
    FIFO(&resource_count, processes);
    for (int i = 0; i < resource_count.process_count; i++) {
        free(processes[i].held_resource);  // free held resources
        for (int j = 0; j < MAX_TASK_REQUESTS; j++) {
            free(processes[i].all_requests[j].request_type);  // free char array in requests
        }
        free(processes[i].all_requests);  // free requests array
        free(processes[i].initial_claim);
    }
    fclose(file_pointer);
    return processes;
}

Process *initalize_Banker(char **argv) {
    FILE *file_pointer;
    file_pointer = fopen(argv[1], "r");
    resource resource_count;

    char resource_count_string[MAX_LINE_LENGTH];  // read first line and get info
    fgets(resource_count_string, sizeof(resource_count), file_pointer);
    resource_count_string[sizeof(resource_count_string) - 1] = '\0';
    char *token;
    int number;
    int store_numbers[MAX_INITIAL_UNITS];
    int store_numbers_pointer = 0;
    token = strtok(resource_count_string, " ");
    while (token != NULL && token[0] != ' ') {
        // Convert the token to an integer using sscanf
        if (sscanf(token, "%d", &number) == 1) {
            // insert into store_numbers_array
            store_numbers[store_numbers_pointer] = number;
            store_numbers_pointer++;
        }
        // Get the next token
        token = strtok(NULL, " ");
    }  // this block is for reading the first line in the file and placing values into the store_numbers array
    if (store_numbers_pointer < 3) {
        printf("Not enough arguments in input\n");
        return NULL;
    }
    resource_count.process_count = store_numbers[0];   // place number of processes
    resource_count.resource_types = store_numbers[1];  // number of resource types
    int j = 2;
    int resource_types_array[resource_count.resource_types];  // placing the resource amounts for each type
    for (int i = 0; i < resource_count.resource_types; i++) {
        resource_types_array[i] = store_numbers[j];
        j++;
    }
    resource_count.resource_available = resource_types_array;                    // reference the array in resource count
    Process *processes = calloc(resource_count.process_count, sizeof(Process));  // all processes
    for (int i = 0; i < resource_count.process_count; i++) {                     // get all request for each process and store into memory
        read_request *requests = (read_request *)calloc(MAX_TASK_REQUESTS, sizeof(read_request));
        int current_request = 0;
        while (1) {  // keep reading until terminated
            char *request_type = calloc(MAX_REQUEST_SIZE, sizeof(char));
            fscanf(file_pointer, "%s %d %d %d", request_type, &requests[current_request].process_id, &requests[current_request].resource_type, &requests[current_request].resource_units);
            requests[current_request].request_type = request_type;
            processes[i].pid = requests[current_request].process_id;
            current_request++;
            if (check_terminated(request_type)) {
                break;
            }
        }
        processes[i].is_terminated = 0;
        processes[i].was_aborted = 0;
        processes[i].is_waiting = 0;
        processes[i].held_resource = calloc(resource_count.resource_types, sizeof(int));
        processes[i].time_taken = 0;
        processes[i].time_waiting = 0;
        processes[i].current_request_pointer = 0;
        processes[i].request_size = current_request;
        processes[i].all_requests = requests;
        processes[i].currently_computing = 0;
        processes[i].compute_cycles = 0;
        processes[i].initial_claim = calloc(resource_count.resource_types, sizeof(int));
    }

    number_of_processes = resource_count.process_count;
    Banker(&resource_count, processes);
    for (int i = 0; i < resource_count.process_count; i++) {
        free(processes[i].held_resource);  // free held resources
        for (int j = 0; j < MAX_TASK_REQUESTS; j++) {
            free(processes[i].all_requests[j].request_type);  // free char array in requests
        }
        free(processes[i].all_requests);  // free requests array
        free(processes[i].initial_claim);
    }
    fclose(file_pointer);
    return processes;
}

int check_all_terminated(resource *resource_count, Process processes[]) {
    for (int i = 0; i < resource_count->process_count; i++) {
        if (processes[i].is_terminated == 0) {
            return 0;
        }
    }
    return 1;
}