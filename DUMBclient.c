#include "DUMBclient.h"
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define MAX_TRIES 3
#define COMMAND_COUNT 7
#define MAX_INPUT 256

typedef struct command {
    char* name;
    char* server_name;
    int key;
    int arg_count;
    char* messages[];
} command;

static unsigned short COMMAND_ARGS[COMMAND_COUNT] = {0, 1, 1, 0, 1, 1, 1};
static char* COMMANDS[COMMAND_COUNT] = {"quit","create","open","next","put","delete","close"};
static char* SERVER_COMMANDS[COMMAND_COUNT] = {"GDBYE", "CREAT", "OPNBX", "NXTMG", "PUTMG", "DELBX", "CLSBX"};
static char* SERVER_RESPONSES[8] = {"OK!", "ER:WHAT?", "ER:EXIST", "ER:NEXST", "ER:OPEND", "ER:NOOPN", "ER:NOTMT", "ER:EMPTY"};
static char* INTERPRET_RESPONSES[8] = {"Success!", "Your command is in some way broken or malformed.", "Box already exists.", "Box does not exist.", "Box is currently in use.", "Box is not open.", "Box is not empty.", "Box is empty."};

char* get_input(char* prompt) {
    char* input_buffer = malloc(sizeof(char) * MAX_INPUT);
    printf("%s> ", prompt);
    fgets(input_buffer, MAX_INPUT, stdin);
    input_buffer[strcspn(input_buffer, "\r\n")] = 0;
    //printf("Input: %s\n", input_buffer);
    return input_buffer;
}

char* get_send_string_default(int command_key) {
    char* input = malloc(sizeof(char) * MAX_INPUT);
    printf("Okay, which box would you like to %s?\n", COMMANDS[command_key]);
    input = get_input(COMMANDS[command_key]);
    char* arg = malloc(sizeof(char) * (strlen(input) + 1));
    strcpy(arg, " ");
    strcpy(arg + 1, input);
    //printf("Returning arg: %s\n", arg);
    return arg;
}

//Small helper fn
int get_number_of_digits(int i) {
    int num = i;
    int digits = 0;
    while (num > 0) {
        num /= 10;
        digits++;
    }
    return digits;
}

char* get_send_string_put(int command_key) {
    char* input = malloc(sizeof(char) * MAX_INPUT);
    printf("Okay, what message would you like to put?\n");
    input = get_input("put");
    int input_size = (strlen(input));
    int number_of_digits = get_number_of_digits(input_size);
    char* arg = malloc(sizeof(char) * (number_of_digits + 2 + input_size));
    strcpy(arg, "!");
    char* size_string = malloc(sizeof(char) * number_of_digits);
    sprintf(size_string, "%d", input_size);
    strcpy(arg + 1, size_string);
    strcpy(arg + 1 + number_of_digits, "!");
    strcpy(arg + 2 + number_of_digits, input);
    return arg;
}

static char* (*COMMAND_ARG_FNS[COMMAND_COUNT])(int) = {NULL, get_send_string_default, get_send_string_default, NULL, get_send_string_put, get_send_string_default, get_send_string_default};

void print_commands() {
    int i = 0;
    for (i; i < COMMAND_COUNT; i++) {
        printf("Command: %s | Arguments: %d\n", COMMANDS[i], COMMAND_ARGS[i]);
    }
    return;
}

int validate_command(char* input) {
    if (strcmp(input, "help") == 0) {
        print_commands();
        return -1;
    }
    int i = 0;
    for (i; i < COMMAND_COUNT; i++) {
        if (strcmp(COMMANDS[i], input) == 0) {
            break;
        }
    }
    return i;
}

void print_message(char* buffer) {
    if (strchr(buffer, '!') == strrchr(buffer, '!')) {
        //This means it coming back from put.
        printf("Message successfully put.\n");
    } else {
        printf("Message: %s\n", strrchr(buffer, '!') + 1);
    }
}

void handle_response(int sock) {
    char* buffer = malloc(sizeof(char) * MAX_INPUT);
    int result = read(sock, buffer, MAX_INPUT);
    if (result == 0) {
        printf("Socket closed.\n");
        exit(0);
    }
    int i = 0;
    for (i; i < 8; i++) {
        if (strcmp(SERVER_RESPONSES[i], buffer) == 0) {
            printf("%s\n", INTERPRET_RESPONSES[i]);
            return;
        }
    }
    //At this point, this means that it was a message that was receieved.
    print_message(buffer);
    //printf("Response: %s\n", buffer);
}

void command_handler_default(int command_key, int sock) {
    //Retrieve the arguments
    char* arg = malloc(sizeof(char) * MAX_INPUT);
    char* (*function_pointer)(int) = COMMAND_ARG_FNS[command_key];
    if (*function_pointer != NULL) {
        arg = function_pointer(command_key);
    }
    //printf("The argument is: %s with length %zu.\n", arg, strlen(arg));
    //At this point, we have the arguments and the command, now we send this information to the server, and see what happens.
    int arg_length = strlen(arg);
    char* send_string = malloc(sizeof(char) * (5 + arg_length));
    //printf("Total length: %d\n", 6 + arg_length);
    strcpy(send_string, SERVER_COMMANDS[command_key]);
    strcpy(send_string + 5, arg);
    send(sock, send_string, 5 + arg_length, 0);
}

void no_argument_command(int command_key, int sock) {
    send(sock, SERVER_COMMANDS[command_key], 5, 0);
}

//ARray of function pointers for each command
static void (*COMMAND_FNS[COMMAND_COUNT])(int, int) = {no_argument_command, command_handler_default, command_handler_default, no_argument_command, command_handler_default, command_handler_default, command_handler_default};

int init_connection(char* ip_address, char* port) {
    int tries = 0;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < -1) {
        printf("Error creating socket. Exiting...\n");
        exit(0);
    }
    printf("Created socket successfully.\nAttempting to connect to %s:%s...\n", ip_address, port);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(port));
    inet_pton(AF_INET, ip_address, &server_address.sin_addr);
    while (tries < MAX_TRIES) {
        if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            printf("Failed to connect.. Reattempting...\n");
        } else {
            send(sock, "HELLO", 5, 0);
            char* buffer = malloc(sizeof(char) * MAX_INPUT);
            int result = read(sock, buffer, MAX_INPUT, 0);
            //printf("Server Response: %s | length: %zu.\n", buffer, strlen(buffer));
            if (strcmp(buffer, "HELLO DUMBv0 ready!") == 0) {
                printf("Successfully connected.\n");
            } else {
                printf("An error occurred. Exiting.\n");
                exit(0);
            }
            free(buffer);
            return sock;
        }
        tries++;
    }
    printf("Failed to create socket after %d tries.\n", MAX_TRIES);
    exit(0);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Error: Need to provide the IP-address and Port Number as arguments.\n");
        exit(0);
    }
    //Try to call the server three times.
    int sock = init_connection(argv[1], argv[2]);
    while (sock) {
        int command_key = validate_command(get_input(">"));
        if (command_key == -1) continue; //This means that the help command was called, continue to next command.
        if (command_key < 7) {
            //We know what command it is because the index is the command.
            //Call the command handler and pass in the index (command key).
            void (*function_pointer)(int, int) = COMMAND_FNS[command_key];
            function_pointer(command_key, sock);
            handle_response(sock);
        } else {
            printf("That is not a command, for a command list enter 'help'\n");
        }
    }
    return 0;
}

