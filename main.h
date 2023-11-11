/**
 * Full Name: Jay Raut
 * Course ID: Section B
 * Description: banker header file
 */

#define MAX_VALUE 0x7FFFFFFF
#define MAX_LINE_LENGTH 100
#define MAX_INITIAL_UNITS 100
#define MAX_TASK_REQUESTS 50
#define MAX_REQUEST_SIZE 50
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int number_of_processes;
typedef struct read_request {
    char *request_type;
    int process_id;
    int resource_type;
    int resource_units;
} read_request;
