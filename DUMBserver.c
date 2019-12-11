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
    setup(8000);
    return 0;
}

void setup(int port) {
    int success = 0;
    // Notify that we are spawning the server
    printf("Spawning server on port %d\n", port);
    // Initialize the connection description
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    int addressLength = sizeof(address);
    // Create the socket to the port
    int socketServer = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServer == 0) { // Verify that we were able to create a socket
        printf("Error: Failed to initialze socket\n");
        exit(1);
    }
    // Set the socket's option value to out of band
    int optionValue = 1; // Gives us out of band data in the normal input queue
    success = setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optionValue, sizeof(optionValue));
    if (success == 1) {
        printf("Error: Failed to set the socket option value\n");
        exit(1);
    }
    // Bind the socket to a particular port
    success = bind(socketServer, (struct sockaddr *)&address, sizeof(address));
    if (success < 0) {
        printf("Error: Failed to bind socket\n");
        exit(1);
    }
    // Listen
    success = listen(socketServer, 3);
    if (success < 0) {
        printf("Error: Failed to listen\n");
        exit(1);
    }
    int connection = 0;
    connection = accept(socketServer, (struct sockaddr*)&address, (socklen_t*)&addressLength);
    if (connection < 0) {
        printf("Error: Failed to accept new connection");
        exit(1);
    }
}
