// Import libraries
#include "DUMBserver.h"

/**
 * Server application entry point
 */
int main(int argc, char** argv) {
    // Verify the number of inputs
    if (argc != 2) {
        printf("Error: Invalid number of arguments. Expecting port number\n");
        return 1;
    }
    // Since we have the number of inputs verified, lets parse through them
    char* portString = argv[1];
    // Create the address
    struct sockaddr_in address = createAddress(8000);
    // Construct the socket
    int masterSocket = socketSetup(address);
    if (masterSocket < 0) {
        printf("Error: Failed to initialize socket\n");
        exit(1);
        return -1;
    }
    socketLoop(address, masterSocket);
    return 0;
}

struct sockaddr_in createAddress(int port) {
    struct sockaddr_in _return;
    _return.sin_family = AF_INET;
    _return.sin_addr.s_addr = INADDR_ANY;
    _return.sin_port = htons(port);
    return _return;
}

int socketSetup(struct sockaddr_in address) {
    int success = 0;
    // Notify that we are spawning the server
    printf("Spawning server\n");
    // Create the socket to the port
    int socketServer = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServer == 0) { // Verify that we were able to create a socket
        printf("Error: Failed to initialze socket\n");
        return -1;
    }
    // Set the socket's option value to out of band
    int optionValue = 1; // Gives us out of band data in the normal input queue
    success = setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optionValue, sizeof(optionValue));
    if (success == 1) {
        printf("Error: Failed to set the socket option value\n");
        return -1;
    }
    // Bind the socket to a particular port
    success = bind(socketServer, (struct sockaddr *)&address, sizeof(address));
    if (success < 0) {
        printf("Error: Failed to bind socket\n");
        return -1;
    }
    // Listen
    success = listen(socketServer, 3);
    if (success < 0) {
        printf("Error: Failed to listen\n");
        return -1;
    }
    return socketServer;
}

void socketLoop(struct sockaddr_in address, int masterSocket) {
    int addressLength = sizeof(address);
    while (1) {
        int connection = 0;
        connection = accept(masterSocket, (struct sockaddr*)&address, (socklen_t*)&addressLength);
        if (connection < 0) {
            printf("Error: Failed to accept new connection");
            exit(1);
        } else {
            //printf("Received client connection\n");
            printEvent(connection, "Connected");
            // Read the incoming connection
            char buffer[1024] = {0};
            read(connection, buffer, 1024);
            // Create the child thread
            pthread_t child;
            // Construct the necessary inputs to this child thread
            ConnectionInput* conIn = (ConnectionInput*)malloc(sizeof(ConnectionInput));
            conIn->id = &child;
            conIn->connection = connection;
            conIn->buffer = buffer;
            // Create the thread to handle this connection
            pthread_create(&child, NULL, connectionWorker, (void*)conIn);
        }
    }
}


void _print(FILE* stream, int socket, char* message) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    getpeername(socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    char str[INET6_ADDRSTRLEN];
    inet_ntop( AF_INET, &address.sin_addr, str, INET6_ADDRSTRLEN);
    time_t now = time(NULL);
    struct tm* now_tm = localtime(&now);
    fprintf(stream, "%02d:%02d %02d %s %s %s\n", now_tm->tm_hour, now_tm->tm_min, now_tm->tm_mday, MONTH[now_tm->tm_mon], str, message);
}

void printEvent(int socket, char* message) {
    _print(stdout, socket, message);
}

void printError(int socket, char* message) {
    _print(stderr, socket, message);
}


void* connectionWorker(void* vargp) {
    ConnectionInput* input = (ConnectionInput*) vargp;
    int connection = input->connection;
    char* buffer = (char*)malloc(sizeof(char) * 1024);
    int valread = -1;
    while(valread != 0) {
        valread = read(connection, buffer, 1024);
        int i = 0;
        while (i < valread) {
            printEvent(connection, buffer + i);
            i += strlen(buffer) + 1;
        }
    }
    if (valread == 0) {
        printEvent(connection, "Disconnected");
    }
}
