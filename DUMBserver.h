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

typedef struct connectionInput {
    pthread_t* id;
    int connection;
    char* buffer;
} ConnectionInput;

struct sockaddr_in createAddress(int port);
int socketSetup(struct sockaddr_in address);
void socketLoop(struct sockaddr_in address, int mainSocket);
void* connectionWorker(void* vargp);

#endif  // DUMBSERVER_H_
