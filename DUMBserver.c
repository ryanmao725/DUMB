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

void* connectionWorker(void* vargp) {
    ConnectionInput* input = (ConnectionInput*) vargp;
    int connection = input->connection;
    char* buffer = (char*)malloc(sizeof(char) * 1024);
    printf("From %d, Received %s\n", connection, input->buffer);
    while(1) {
        int valread = read(connection, buffer, 1024);
        printf("From Thread %d: %s\n", valread, buffer);
    }
}
