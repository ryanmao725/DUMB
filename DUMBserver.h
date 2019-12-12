/**
 * @author Shivan/Ryan
 * Copyright 2019 Shivan/Ryan
 */
#ifndef DUMBSERVER_H_
#define DUMBSERVER_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

typedef struct messageQueue {
    int length;
    char* message;
    struct messageQueue* next;
} Message;

typedef struct messageBox {
    char* name;
    Message* messages;
    int isOpen;
    struct messageBox* next;
} MessageBox;

MessageBox* boxes = NULL;

pthread_mutex_t* boxesLock = NULL;

typedef struct connectionInput {
    pthread_t* id;
    int connection;
    char* buffer;
} ConnectionInput;

char* MONTH[12] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

struct sockaddr_in createAddress(int port);
int socketSetup(struct sockaddr_in address);
void socketLoop(struct sockaddr_in address, int mainSocket);
void* connectionWorker(void* vargp);

void printEvent(int socket, char* message);
void printError(int socket, char* message);
void _print(FILE* stream, int socket, char* message);

void respond(int socket, char* message);

MessageBox* boxCreate(int connection, char* buffer, MessageBox* OPEN_BOX);
MessageBox* boxOpen(int connection, char* buffer, MessageBox* OPEN_BOX);
MessageBox* boxPut(int connection, char* buffer, MessageBox* OPEN_BOX);
MessageBox* boxGet(int connection, char* buffer, MessageBox* OPEN_BOX);
MessageBox* boxDelete(int connection, char* buffer, MessageBox* OPEN_BOX);
MessageBox* boxClose(int connection, char* buffer, MessageBox* OPEN_BOX);

#endif  // DUMBSERVER_H_
