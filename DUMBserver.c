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
        if (strlen(buffer) > 0) {
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
                    if (OPEN_BOX != NULL) {
                        OPEN_BOX->isOpen = 0;
                    }
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
        } else {
            valread = 0;
        }
    }
    // The client disconnected
    if (valread == 0) {
        printEvent(connection, "Disconnected");
        if (OPEN_BOX != NULL) {
            OPEN_BOX->isOpen = 0;
        }
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
                    printEvent(connection, "> OK!");
                    respond(connection, "OK!");
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
                printEvent(connection, "> OK!");
                respond(connection, "OK!");
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
                // Verify that this box exists already
                MessageBox* ptr = boxes;
                while (ptr != NULL) {
                    if (strcmp(ptr->name, name) == 0) {
                        // We have a match! Verify that this is not opened by anyone else
                        if (ptr->isOpen == 1) {
                            printError(connection, "> ER:OPEND");
                            respond(connection, "ER:OPEND");
                            return OPEN_BOX;
                        } else {
                            // Verify that there isn't an open box
                            if (OPEN_BOX != NULL) {
                                printError(connection, "> ER:NCLSD");
                                respond(connection, "ER:NCLSD");
                                return OPEN_BOX;
                            }
                            ptr->isOpen = 1;
                            printEvent(connection, "> OK!");
                            respond(connection, "OK!");
                            return ptr;
                        }
                    }
                    ptr = ptr->next;
                }
                // If we didn't find anything, we know that this doesn't exist
                printError(connection, "> ER:NEXST");
                respond(connection, "ER:NEXST");
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
    // Check if we have an open box
    if (OPEN_BOX == NULL) {
        printError(connection, "> ER:NOOPN");
        respond(connection, "ER:NOOPN");
        return OPEN_BOX;
    }
    // Check to see if we have any messages
    if (OPEN_BOX->messages == NULL) {
        printError(connection, "> ER:EMPTY");
        respond(connection, "ER:EMPTY");
        return OPEN_BOX;
    }
    Message* msg = OPEN_BOX->messages;
    OPEN_BOX->messages = OPEN_BOX->messages->next;
    char* length = (char*)malloc(sizeof(char) * 20);
    sprintf(length, "%d", msg->length);
    char* output = (char*)malloc(sizeof(char) * (4 + strlen(length) + strlen(msg->message)));
    strcpy(output, "OK!");
    strcpy(output + 3, length);
    strcpy(output + 3 + strlen(length), "!");
    char* ms = (char*)malloc(sizeof(char) * msg->length);
    strncpy(ms, msg->message, msg->length);
    strcpy(output + 4 + strlen(length), ms);
    char* logOut = (char*)malloc(sizeof(char) * (6 + strlen(length) + strlen(msg->message)));
    strcpy(logOut, "> ");
    strcpy(logOut + 2, output);
    printEvent(connection, logOut);
    respond(connection, output);
    return OPEN_BOX;
}
MessageBox* boxPut(int connection, char* buffer, MessageBox* OPEN_BOX) {
    // Verify that there is an argument
    if (*(buffer + 5) == '!') {
        char* arguments = buffer + 6;
        if (arguments[0] == '!') {
            printError(connection, "> ER:WHAT?");
            respond(connection, "ER:WHAT?");
            return OPEN_BOX;
        }
        char* message = (char*)malloc(sizeof(char) * strlen(arguments));
        strcpy(message, arguments);
        char* length = strtok(message, "!");
        if (strlen(length) == 0) {
            printError(connection, "> ER:WHAT?");
            respond(connection, "ER:WHAT?");
            return OPEN_BOX;
        }
        message = arguments + strlen(length);
        if (message[0] == '!') {
            // Extract the message
            char* mes = (char*)malloc(sizeof(char) * atoi(length));
            int len = atoi(length);
            message = message + 1;
            strncpy(mes, message, len);
            // Add to the queue
            if (OPEN_BOX == NULL) {
                // Verify that the box is open
                printError(connection, "> ER:NOOPN");
                respond(connection, "ER:NOOPN");
                return OPEN_BOX;
            }
            Message* m = (Message*)malloc(sizeof(Message));
            m->length = len;
            m->message = mes;
            m->next = OPEN_BOX->messages;
            OPEN_BOX->messages = m;
            char* output = (char*)malloc(sizeof(char) * (3 + strlen(length)));
            strcpy(output, "OK!");
            strcpy(output + 3, length);
            char* logOut = (char*)malloc(sizeof(char) * (5 + strlen(length)));
            strcpy(logOut, "> ");
            strcpy(logOut + 2, output);
            printEvent(connection, logOut);
            respond(connection, output);
            return OPEN_BOX;
        }
    }
    printError(connection, "> ER:WHAT?");
    respond(connection, "ER:WHAT?");
    return OPEN_BOX;
}
MessageBox* boxDelete(int connection, char* buffer, MessageBox* OPEN_BOX) {
    // Verify that there is an argument
    if (*(buffer + 5) == ' ') {
        char* name = buffer + 6;
        // Verify that the length is between 5 and 25
        if (strlen(name) >= 5 && strlen(name) <= 25) {
            // Verify that it starts with an alphabetical character
            char c = (char)name[0];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                // Verify that this box exists already
                MessageBox* ptr = boxes;
                MessageBox* parent = NULL;
                while (ptr != NULL) {
                    if (strcmp(ptr->name, name) == 0) {
                        // We have a match! Verify that this is not opened by anyone else
                        if (ptr->isOpen == 1) {
                            printError(connection, "> ER:OPEND");
                            respond(connection, "ER:OPEND");
                            return OPEN_BOX;
                        } else {
                            // Verify that the messages are empty
                            if (ptr->messages != NULL) {
                                printError(connection, "> ER:NOTMT");
                                respond(connection, "ER:NOTMT");
                                return OPEN_BOX;
                            }
                            // We can safely delete
                            if (boxes == ptr) {
                                boxes = ptr->next;
                            } else {
                                parent->next = ptr->next;
                            }
                            printEvent(connection, "> OK!");
                            respond(connection, "OK!");
                            return OPEN_BOX;
                        }
                    }
                    parent = ptr;
                    ptr = ptr->next;
                }
                // If we didn't find anything, we know that this doesn't exist
                printError(connection, "> ER:NEXST");
                respond(connection, "ER:NEXST");
                return OPEN_BOX;
            }
        }
    }
    // Malformed input
    printError(connection, "> ER:WHAT?");
    respond(connection, "ER:WHAT?");
    return OPEN_BOX;

}
MessageBox* boxClose(int connection, char* buffer, MessageBox* OPEN_BOX) {
    // Verify that there is an argument
    if (*(buffer + 5) == ' ') {
        if (OPEN_BOX == NULL) {
            printError(connection, "> ER:NOOPN");
            respond(connection, "ER:NOOPN");
            return OPEN_BOX;
        }
        char* name = buffer + 6;
        // Verify that the length is between 5 and 25
        if (strlen(name) >= 5 && strlen(name) <= 25) {
            // Verify that it starts with an alphabetical character
            char c = (char)name[0];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                if (strcmp(OPEN_BOX->name, name) == 0) {
                    if (OPEN_BOX->isOpen == 0) {
                        printError(connection, "> ER:NOOPN");
                        respond(connection, "ER:NOOPN");
                        return OPEN_BOX;
                    } else {
                        OPEN_BOX->isOpen = 0;
                        printEvent(connection, "> OK!");
                        respond(connection, "OK!");
                        return NULL;
                    }
                } else {
                    // In the case we want to close any box regardless of whether the use has it open.....
                    printError(connection, "> ER:NOOPN");
                    respond(connection, "ER:NOOPN");
                    return OPEN_BOX;
                }
            }
        }
    }
    // Malformed input
    printError(connection, "> ER:WHAT?");
    respond(connection, "ER:WHAT?");
    return OPEN_BOX;
}
