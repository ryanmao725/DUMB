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

static command COMMAND_LIST[COMMAND_COUNT];
static char* COMMANDS[COMMAND_COUNT] = {"quit","create","open","next","put","delete","close"};
static char* SERVER_COMMANDS[COMMAND_COUNT] = {"GDBYE", "CREAT", "OPNBX", "NXTMG", "PUTMG", "DELBX", "CLSBX"};
static int COMMAND_ARGS[COMMAND_COUNT] = {0, 1, 1, 0, 2, 1, 1};

char* get_input(char* prompt) {
    char* input_buffer = malloc(sizeof(char) * MAX_INPUT);
    printf("%s> ", prompt);
    fgets(input_buffer, MAX_INPUT, stdin);
    input_buffer[strcspn(input_buffer, "\r\n")] = 0;
    return input_buffer;
}

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

void request_args(int command_key, char* args[], int arg_number){
    if (arg_number == 0) {
        printf("Okay, which box name would you like to %s?\n", COMMANDS[command_key]);
        args[arg_number] = get_input(COMMANDS[command_key]);
    } else {
        printf("Okay, what message would you like to %s?\n", COMMANDS[command_key]);
        args[arg_number] = get_input(COMMANDS[command_key]);
    }
}

int command_handler(int command_key) {
    int arg_count = 0;
    int num_args = COMMAND_ARGS[command_key];
    //Retrieve the arguments
    char* args[num_args];
    while (arg_count < num_args) {
        request_args(command_key, args, arg_count);
        arg_count++;
    }
    //At this point, we have the arguments and the command, now we send this information to the server, and see what happens.
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
            printf("Connected!\n");
            return sock;
        }
        tries++;
    }
    printf("Failed to create socket after %d tries\n.", MAX_TRIES);
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
            send(sock, COMMANDS[command_key], strlen(COMMANDS[command_key]), 0);
            //command_handler(command_key);
        } else {
            printf("That is not a command, for a command list enter 'help'\n");
        }
    }
    return 0;
}

