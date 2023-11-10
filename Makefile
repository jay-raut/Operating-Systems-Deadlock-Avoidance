CC=gcc
DEPS = main.h process.h resource.h
OBJ = main.o check_request_type.o
CFLAGS =-Wall
STD_FLAGS=-std=c99
LIBS = -lm

main.o: main.c main.h process.h resource.h 
	$(CC) $(CFLAGS) main.c $(STD_FLAG)  -o banker $(LIBS)

check_request_type.o: check_request_type.c
	$(CC) $(CFLAGS) -c check_request_type.c

clean:
	rm -rf *.o    

