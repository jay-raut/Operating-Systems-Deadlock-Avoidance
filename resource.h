/**
 * Full Name: Jay Raut
 * Course ID: Section B
 * Description: resource header file for storing resource struct
 */
typedef struct resource {
    int process_count;
    int resource_types;
    int *resource_available;
    FILE *file_pointer;
    char *file_name;
} resource;

