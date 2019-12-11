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

char* get_input(char* prompt) {
    char* input_buffer = malloc(sizeof(char) * MAX_INPUT);
    printf("%s> ", prompt);
    fgets(input_buffer, MAX_INPUT, stdin);
    input_buffer[strcspn(input_buffer, "\r\n")] = 0;
    return input_buffer;
}

char* get_send_string_default(int command_key) {
    char* input = malloc(sizeof(char) * MAX_INPUT);
    printf("Okay, which box would you like to %s?\n", COMMANDS[command_key]);
    input = get_input(COMMANDS[command_key]);
    char* arg = malloc(sizeof(char) * (strlen(input) + 1));
    strcat(arg, " ");
    strcat(arg, input);
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
    strcat(arg, "!");
    char* size_string = malloc(sizeof(char) * number_of_digits);
    sprintf(size_string, "%d", input_size);
    strcat(arg, size_string);
    strcat(arg, "!");
    strcat(arg, input);
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

int command_handler(int command_key, int sock) {
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
    //printf("The string being sent to the server is: %s.\n", send_string);
    send(sock, send_string, 5 + arg_length, 0);
    char* buffer = malloc(sizeof(char) * MAX_INPUT);
    int result = read(sock, buffer, MAX_INPUT);
    printf("Server Response: %s.\n", buffer);
    if (result == -1) exit(0);
    return 0;
}

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
            command_handler(command_key, sock);
        } else {
            printf("That is not a command, for a command list enter 'help'\n");
        }
    }
    return 0;
}

