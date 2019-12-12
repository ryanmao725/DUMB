/**
 * @author Shivan/Ryan
 * Copyright 2019 Shivan/Ryan
 */
#ifndef DUMBCLIENT_H_
#define DUMBCLIENT_H_

/******************************************************************************/
/* INCLUDES *******************************************************************/
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
/******************************************************************************/
/* INCLUDES *******************************************************************/
/******************************************************************************/

/******************************************************************************/
/* MACROS *********************************************************************/
/******************************************************************************/
#define MAX_TRIES 3
#define COMMAND_COUNT 7
#define MAX_INPUT 256
/******************************************************************************/
/* MACROS *********************************************************************/
/******************************************************************************/

/******************************************************************************/
/* STRUCTURES *****************************************************************/
/******************************************************************************/
typedef struct command {
    char* name;
    char* server_name;
    int key;
    int arg_count;
    char* messages[];
} command;
/******************************************************************************/
/* STRUCTURES *****************************************************************/
/******************************************************************************/

/******************************************************************************/
/* VARIABLES ******************************************************************/
/******************************************************************************/
static unsigned short COMMAND_ARGS[COMMAND_COUNT] = {
    0,
    1,
    1,
    0,
    1,
    1,
    1
};
static char* COMMANDS[COMMAND_COUNT] = {
    "quit",
    "create",
    "open",
    "next",
    "put",
    "delete",
    "close"
};
static char* SERVER_COMMANDS[COMMAND_COUNT] = {
    "GDBYE",
    "CREAT",
    "OPNBX",
    "NXTMG",
    "PUTMG",
    "DELBX",
    "CLSBX"
};
static char* SERVER_RESPONSES[10] = {
    "OK!",
    "ER:WHAT?",
    "ER:EXIST",
    "ER:NEXST",
    "ER:OPEND",
    "ER:NOOPN",
    "ER:NOTMT",
    "ER:EMPTY",
    "ER:INVCL",
    "ER:NCLSD",
};
static char* INTERPRET_RESPONSES[10] = {
    "Success!",
    "Your command is in some way broken or malformed.",
    "Box already exists.",
    "Box does not exist.",
    "Box is currently in use.",
    "Box is not open.",
    "Box is not empty.",
    "Box is empty.",
    "Invalid client.",
    "Box is not closed."
};
/******************************************************************************/
/* VARIABLES ******************************************************************/
/******************************************************************************/


/******************************************************************************/
/* FUNCTIONS ******************************************************************/
/******************************************************************************/
char* get_input(char* prompt);
char* get_send_string_default(int command_key);
int get_number_of_digits(int i);
char* get_send_string_put(int command_key);
void print_commands();
int validate_command(char* input);
void print_message(char* buffer);
void handle_response(int sock);
void command_handler_default(int command_key, int sock);
void no_argument_command(int command_key, int sock);
int init_connection(char* ip_address, char* port);
/******************************************************************************/
/* FUNCTIONS ******************************************************************/
/******************************************************************************/
#endif  // DUMBCLIENT_H_
