#include "DUMBclient.h"
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#define PORT 8000

int main(int argc, char* argv[]) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < -1) {
        printf("Socket creation failed. Exiting...\n");
        exit(0);
    }
    printf("Socket creation successful.\n");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    char* input_buffer = malloc(sizeof(char) * 256);
    while(1) {
        fgets(input_buffer, 256, stdin);
        input_buffer[strcspn(input_buffer,"\r\n")] = 0;
        send(sock, "input_buffer", 256, 0);
    }
    return 0;
}
