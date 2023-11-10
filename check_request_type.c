int check_initiate(char *request_type) {
    char initiate[] = "initiate";
    for (int i = 0; i < sizeof(initiate) / sizeof(char); i++) {
        if (request_type[i] != initiate[i]) {
            return 0;
        }
    }
    return 1;
}
int check_terminated(char *request_type) {
    char terminated[] = "terminate";

    for (int i = 0; i < sizeof(terminated) / sizeof(char); i++) {
        if (request_type[i] != terminated[i]) {
            return 0;
        }
    }
    return 1;
}
int check_request(char *request_type) {
    char request[] = "request";
    for (int i = 0; i < sizeof(request) / sizeof(char); i++) {
        if (request_type[i] != request[i]) {
            return 0;
        }
    }
    return 1;
}
int check_release(char *request_type) {
    char release[] = "release";
    for (int i = 0; i < sizeof(release) / sizeof(char); i++) {
        if (request_type[i] != release[i]) {
            return 0;
        }
    }
    return 1;
}
int check_compute(char *request_type) {
    char compute[] = "compute";
    for (int i = 0; i < sizeof(compute) / sizeof(char); i++) {
        if (request_type[i] != compute[i]) {
            return 0;
        }
    }
    return 1;
}
