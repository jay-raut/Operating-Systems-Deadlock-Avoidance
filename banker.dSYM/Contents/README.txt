To compile type 'make' into the terminal. A binary called 'banker' will be created.

main.c contains the algorithms FIFO and Banker and their helper methods which set them up to run. Their helper methods will parse the input file taking in all of the requests for each 
process and place them into their respective processes. 

check_request_type.c contains functions which check the request type from the file which is used to determine what type of task is to be ran. 

process.h and resource.h contains the structs used for the methods. They contain the definition of a process and resource types. 

Usage: ./banker 'name_of_input_file' (without quotes)