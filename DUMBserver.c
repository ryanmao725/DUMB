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
    int port = atoi(portString);
    if (port < 1000) {
        printf("Error: Invalid port number. Port must be greater than 1000\n");
        return 1;
    }
    // Create the address
    struct sockaddr_in address = createAddress(port);
    // Construct the socket
    int masterSocket = socketSetup(address);
    if (masterSocket < 0) {
        printf("Error: Failed to initialize socket\n");
        exit(1);
        return -1;
    }
    // Start the main worker loop
    socketLoop(address, masterSocket);
    return 0;
}

struct sockaddr_in createAddress(int port) {
    // Create the socket structure for later use
    struct sockaddr_in _return;
    _return.sin_family = AF_INET;
    _return.sin_addr.s_addr = INADDR_ANY;
    _return.sin_port = htons(port);
    return _return;
}

int socketSetup(struct sockaddr_in address) {
    int success = 0;
    // Notify that we are spawning the server
    printf("Spawning server on port %d\n", ntohs(address.sin_port));
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
    // Get a hold of the address size
    int addressLength = sizeof(address);
    // Begin the main worker loop
    while (1) {
        // Wait for someone to connect
        int connection = 0;
        connection = accept(masterSocket, (struct sockaddr*)&address, (socklen_t*)&addressLength);
        if (connection < 0) {
            printf("Error: Failed to accept new connection");
            exit(1);
        } else {
            // Read the incoming connection
            char buffer[1024] = {0};
            read(connection, buffer, 1024);
            // Verify that we have a DUMB client connected
            if (strcmp(buffer, "HELLO") == 0) {
                printEvent(connection, "Connected");
                // Create the child thread
                pthread_t child;
                // Construct the necessary inputs to this child thread
                ConnectionInput* conIn = (ConnectionInput*)malloc(sizeof(ConnectionInput));
                conIn->id = &child;
                conIn->connection = connection;
                conIn->buffer = buffer;
                // Create the thread to handle this connection
                pthread_create(&child, NULL, connectionWorker, (void*)conIn);
                respond(connection, "HELLO DUMBv0 ready!");
            } else {
                // This client couldn't connect properly since it was invalid
                printError(connection, "ER:INVCL");
                respond(connection, "ER:INVCL\0");
                close(connection);
            }
        }
    }
}


void _print(FILE* stream, int socket, char* message) {
    // Get the address
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    getpeername(socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    // Get the ipaddress of the client
    char str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET, &address.sin_addr, str, INET6_ADDRSTRLEN);
    // Get the current time information
    time_t now = time(NULL);
    struct tm* now_tm = localtime(&now);
    // Print it out with the requested stream
    fprintf(stream, "%02d:%02d %02d %s %s %s\n", now_tm->tm_hour, now_tm->tm_min, now_tm->tm_mday, MONTH[now_tm->tm_mon], str, message);
}

void printEvent(int socket, char* message) {
    // Print to STDOUT
    _print(stdout, socket, message);
}

void printError(int socket, char* message) {
    // Print to STDERR
    _print(stderr, socket, message);
}

void respond(int socket, char* message) {
    // Respond the the client
    send(socket, message, strlen(message), 0);
}

void* connectionWorker(void* vargp) {
    // Parse through the inputs
    ConnectionInput* input = (ConnectionInput*) vargp;
    int connection = input->connection;
    // Initialize our read variable
    int valread = -1;
    // The next line should ALWAYS print out ""HELLO""
    printEvent(connection, input->buffer);
    // Respond to inputs as they come!
    MessageBox* OPEN_BOX = NULL;
    while(valread != 0) {
        char* buffer = (char*)malloc(sizeof(char) * 1024);
        valread = read(connection, buffer, 1024);
        printEvent(connection, buffer);
        // Read the first 5 characters to extract the command
        char cmd[5];
        strncpy(cmd, buffer, 5);
        if (strlen(cmd) != 5) {
            printError(connection, "> ER:WHAT?");
            respond(connection, "ER:WHAT?");
        } else {
            if (strcmp(cmd, "HELLO") == 0) {
                printEvent(connection, "> HELLO DUMBv0 ready!");
                respond(connection, "HELLO DUMBv0 ready!");
            } else if (strcmp(cmd, "GDBYE") == 0) {
                printEvent(connection, "> Killed");
                close(connection);
                valread = 0;
            } else if (strcmp(cmd, "CREAT") == 0) {
                OPEN_BOX = boxCreate(connection, buffer, OPEN_BOX);
            } else if (strcmp(cmd, "OPNBX") == 0) {
                OPEN_BOX = boxOpen(connection, buffer, OPEN_BOX);
            } else if (strcmp(cmd, "NXTMG") == 0) {
                OPEN_BOX = boxGet(connection, buffer, OPEN_BOX);
            } else if (strcmp(cmd, "PUTMG") == 0) {
                OPEN_BOX = boxPut(connection, buffer, OPEN_BOX);
            } else if (strcmp(cmd, "DELBX") == 0) {
                OPEN_BOX = boxDelete(connection, buffer, OPEN_BOX);
            } else if (strcmp(cmd, "CLSBX") == 0) {
                OPEN_BOX = boxClose(connection, buffer, OPEN_BOX);
            } else {
                printError(connection, "> ER:WHAT?");
                respond(connection, "ER:WHAT?");
            }
        }
    }
    // The client disconnected
    if (valread == 0) {
        printEvent(connection, "Disconnected");
    }
}

MessageBox* boxCreate(int connection, char* buffer, MessageBox* OPEN_BOX) {
    // Verify that there is an argument
    if (*(buffer + 5) == ' ') {
        char* name = buffer + 6;
        // Verify that the length is between 5 and 25
        if (strlen(name) >= 5 && strlen(name) <= 25) {
            // Verify that it starts with an alphabetical character
            char c = (char)name[0];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                // Verify that this doesn't exist already
                MessageBox* ptr = boxes;
                while (ptr != NULL) {
                    printf("%s, %s\n", ptr->name, name);
                    if (strcmp(ptr->name, name) == 0) {
                        // We have a duplicate
                        printError(connection, "> ER:EXIST");
                        respond(connection, "ER:EXIST");
                        return OPEN_BOX;
                    }
                    ptr = ptr->next;
                }
                // Doesn't already exist, lets add it in
                ptr = boxes;
                if (ptr == NULL) {
                    boxes = (MessageBox*)malloc(sizeof(MessageBox));
                    boxes->name = name;
                    boxes->messages = NULL;
                    boxes->next = NULL;
                    boxes->isOpen = 0;
                    printEvent(connection, "> Ok!");
                    respond(connection, "Ok!");
                    return OPEN_BOX;
                }
                while (ptr->next != NULL) {
                    ptr = ptr->next;
                }
                ptr->next = (MessageBox*)malloc(sizeof(MessageBox));
                ptr->next->name = name;
                ptr->next->messages = NULL;
                ptr->next->next = NULL;
                ptr->next->isOpen = 0;
                printEvent(connection, "> Ok!");
                respond(connection, "Ok!");
                return OPEN_BOX;
            }
        }
    }
    // Malformed input
    printError(connection, "> ER:WHAT?");
    respond(connection, "ER:WHAT?");
    return OPEN_BOX;
}
MessageBox* boxOpen(int connection, char* buffer, MessageBox* OPEN_BOX) {
    // Verify that there is an argument
    if (*(buffer + 5) == ' ') {
        char* name = buffer + 6;
        // Verify that the length is between 5 and 25
        if (strlen(name) >= 5 && strlen(name) <= 25) {
            // Verify that it starts with an alphabetical character
            char c = (char)name[0];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                // Verify that this doesn't exist already
                MessageBox* ptr = boxes;
                while (ptr != NULL) {
                    printf("%s, %s\n", ptr->name, name);
                    if (strcmp(ptr->name, name) == 0) {
                        // We have a duplicate
                        printError(connection, "> ER:EXIST");
                        respond(connection, "ER:EXIST");
                        return OPEN_BOX;
                    }
                    ptr = ptr->next;
                }
                // Doesn't already exist, lets add it in
                ptr = boxes;
                if (ptr == NULL) {
                    boxes = (MessageBox*)malloc(sizeof(MessageBox));
                    boxes->name = name;
                    boxes->messages = NULL;
                    boxes->next = NULL;
                    boxes->isOpen = 0;
                    printEvent(connection, "> Ok!");
                    respond(connection, "Ok!");
                    return OPEN_BOX;
                }
                while (ptr->next != NULL) {
                    ptr = ptr->next;
                }
                ptr->next = (MessageBox*)malloc(sizeof(MessageBox));
                ptr->next->name = name;
                ptr->next->messages = NULL;
                ptr->next->next = NULL;
                ptr->next->isOpen = 0;
                printEvent(connection, "> Ok!");
                respond(connection, "Ok!");
                return OPEN_BOX;
            }
        }
    }
    // Malformed input
    printError(connection, "> ER:WHAT?");
    respond(connection, "ER:WHAT?");
    return OPEN_BOX;
}
MessageBox* boxGet(int connection, char* buffer, MessageBox* OPEN_BOX) {
    return OPEN_BOX;
}
MessageBox* boxPut(int connection, char* buffer, MessageBox* OPEN_BOX) {
    return OPEN_BOX;
}
MessageBox* boxDelete(int connection, char* buffer, MessageBox* OPEN_BOX) {
    return OPEN_BOX;
}
MessageBox* boxClose(int connection, char* buffer, MessageBox* OPEN_BOX) {
    return OPEN_BOX;
}
