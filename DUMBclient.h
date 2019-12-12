/**
 * @author Shivan/Ryan
 * Copyright 2019 Shivan/Ryan
 */
#ifndef DUMBCLIENT_H_
#define DUMBCLIENT_H_

#include <stdlib.h>
#include <stdio.h>
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
static char* SERVER_RESPONSES[8] = {
    "OK!",
    "ER:WHAT?",
    "ER:EXIST",
    "ER:NEXST",
    "ER:OPEND",
    "ER:NOOPN",
    "ER:NOTMT",
    "ER:EMPTY"
};
static char* INTERPRET_RESPONSES[8] = {"Success!", "Your command is in some way broken or malformed.", "Box already exists.", "Box does not exist.", "Box is currently in use.", "Box is not open.", "Box is not empty.", "Box is empty."};


#endif  // DUMBCLIENT_H_
