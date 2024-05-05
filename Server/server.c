#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 7777
#define MAX_MSG 100
#define BUFFER_SIZE 1024
#define SUCCESS 0
#define ERROR 1

#define END_LINE '\n'

// Structure for passing client socket information to a thread
typedef struct {
    int socket;
    struct sockaddr_in client_addr;
} ClientInfo;

int read_line(int newSd, char *line_to_return) {
    static int rcv_ptr = 0;
    static char rcv_msg[MAX_MSG];
    static int n;
    int offset = 0;

    while (1) {
        if (rcv_ptr == 0) {
            // Read data from the socket
            memset(rcv_msg, 0x0, MAX_MSG);
            n = recv(newSd, rcv_msg, MAX_MSG, 0);
            if (n < 0) {
                perror("Cannot receive data");
                return ERROR;
            } else if (n == 0) {
                printf("Connection closed by client\n");
                close(newSd);
                return ERROR;
            }
        }

        // Copy bytes to line_to_return until a newline is found
        while (rcv_ptr < n && rcv_msg[rcv_ptr] != END_LINE) {
            line_to_return[offset++] = rcv_msg[rcv_ptr++];
        }

        // If the end of a line is reached
        if (rcv_ptr < n && rcv_msg[rcv_ptr] == END_LINE) {
            line_to_return[offset++] = END_LINE;
            rcv_ptr++;
            return offset;
        }

        // If the buffer is exhausted but no newline was found
        if (rcv_ptr == n) {
            rcv_ptr = 0;
        }
    }
}

// Function that handles communication with a client
void *handle_client(void *arg) {
    ClientInfo *client_info = (ClientInfo *)arg;
    int client_socket = client_info->socket;
    struct sockaddr_in client_addr = client_info->client_addr;
    char line[MAX_MSG];
    char sendBuff[BUFFER_SIZE];

    // Print a message when a client connects
    printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Initialize the line buffer
    memset(line, 0x0, MAX_MSG);

    // Receive data from the client
    while (read_line(client_socket, line) != ERROR) {
        // Print the received message to the server console
        printf("Received from client: %s", line);

        // Respond to the client
        snprintf(sendBuff, sizeof(sendBuff), "Server received: %s", line);
        if (send(client_socket, sendBuff, strlen(sendBuff), 0) < 0) {
            perror("Cannot send data");
            close(client_socket);
            break;
        }

        // Clear the line buffer for the next message
        memset(line, 0x0, MAX_MSG);
    }

    // Close the client socket and free the allocated memory
    close(client_socket);
    free(client_info);
    return NULL;
}

int main(int argc, char *argv[]) {
    int sd, newSd;
    socklen_t cliLen;
    struct sockaddr_in servAddr, cliAddr;
    pthread_t tid;

    // Create a server socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("Cannot open socket");
        return ERROR;
    }

    // Set up the server address
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORT);

    // Bind socket to the address and port
    if (bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("Cannot bind port");
        close(sd);
        return ERROR;
    }

    // Listen for incoming connections
    listen(sd, 5);
    printf("%s: waiting for data on port TCP %u\n", argv[0], PORT);

    while (1) {
        // Accept an incoming connection
        cliLen = sizeof(cliAddr);
        newSd = accept(sd, (struct sockaddr *)&cliAddr, &cliLen);
        if (newSd < 0) {
            perror("Cannot accept connection");
            continue;
        }

        // Allocate and initialize client info
        ClientInfo *client_info = malloc(sizeof(ClientInfo));
        client_info->socket = newSd;
        client_info->client_addr = cliAddr;

        // Create a new thread for each client
        pthread_create(&tid, NULL, handle_client, (void *)client_info);
        pthread_detach(tid); // Detach the thread to allow automatic cleanup
    }

    // Close the server socket
    close(sd);
    return 0;
}
