/**
 * Full Name: Jay Raut
 * Course ID: Section B
 * Description: banker header file
 */

typedef struct read_request {
    char *request_type;
    int process_id;
    int resource_type;
    int resource_units;
} read_request;
