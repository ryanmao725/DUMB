#include "DUMBclient.h"
#include<string.h>
#define COMMAND_COUNT 7
#define MAX_INPUT 256

static char* COMMANDS[COMMAND_COUNT] = {"quit","create","open","next","put","delete","close"};
static int COMMAND_ARGS[COMMAND_COUNT] = {0, 1, 1, 0, 2, 1, 1};

char* get_input(char* prompt) {
    char* input_buffer = malloc(sizeof(char) * MAX_INPUT);
    printf("%s: ", prompt);
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

int command_handler(int command_key) {
    int arg_count = 0;
    int num_args = COMMAND_ARGS[command_key];
    //If the number of arguments is NOT 0, we want to initiate an array containing all arguments
    char* args[num_args];
    while (arg_count < num_args) {
        args[arg_count] = get_input("arg");
        arg_count++;
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Error: Need to provide the IP-address and Port Number as arguments.\n");
        return -1;
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
    char* input_buffer = malloc(sizeof(char) * MAX_INPUT);
    while (connected) {
        int command_key = validate_command(get_input("Input command"));
        if (command_key == -1) continue;
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

