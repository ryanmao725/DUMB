#include "DUMBclient.h"
#include<string.h>
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

int init_client() {
    //This method initializes all the commands and stores them into the array.
    int i = 0;
    for (i; i < COMMAND_COUNT; i++) {

    }

}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Error: Need to provide the IP-address and Port Number as arguments.\n");
        exit(0);
    }
    //Try to call the server three times.
    int connected = 1;
    int tries = 0;
    while (tries < 3 && !connected) {
        //Attempt socket connection here.
        //Need to implement if statement to exit if unsuccessful after three tries.
        printf("Attempting to connect to %s:%s...\n", argv[1], argv[2]);
        tries++;
    }
    if (!connected) {
        printf("Unable to connect, exiting\n");
        exit(0);
    }
    while (connected) {
        int command_key = validate_command(get_input(">"));
        if (command_key == -1) continue; //This means that the help command was called, continue to next command.
        if (command_key < 7) {
            //We know what command it is because the index is the command.
            //Call the command handler and pass in the index (command key).
            command_handler(command_key);
        } else {
            printf("That is not a command, for a command list enter 'help'\n");
        }
    }
    return 0;
}

