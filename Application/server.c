#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "chat.c"

#define PORT 7777
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

#define SUCCESS 0
#define ERROR 1

#define END_LINE '\n'
#define MAX_MSG 100

#define SPACE ' '
#define MAX_TOKENS 4

#define MAX_CHAT 4

// Client struct definition
typedef struct {
    int socket;
    char room[NAME_SIZE];
    char username[BUFFER_SIZE];
} Client;

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

char* decode(char* line[]) {
    char *tokens[MAX_TOKENS];  // Array to hold pointers to tokens
    int n = 0;  // Token counter

    // Get the first token
    char *token = strtok(line, SPACE);

    // Walk through the rest of the tokens
    while (token != NULL) {
        tokens[n++] = token;  // Save the token
        if (n >= MAX_TOKENS) {
            break;  // Prevent exceeding the array bounds
        }
        token = strtok(NULL, SPACE);  // Get the next token
    }

    return token;
}

int main(int argc, char *argv[]) {
    int sd, newSd;
    socklen_t cliLen;
    struct sockaddr_in servAddr, cliAddr;
    char line[MAX_MSG];
    char sendBuff[BUFFER_SIZE];

    Chat* chat_list[MAX_CHAT] = [];

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

        // Print a message when a client connects
        printf("Client connected from %s:%d\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));

        // Initialize the line buffer
        memset(line, 0x0, MAX_MSG);

        // Receive data from the client
        while (read_line(newSd, line) != ERROR) {

            // Print the received message to the server console
            printf("Received from client: %s", line);

            char* command = decode(line);

            if (command[0] == "login") {

            } else if (command[0] == "logout") {

            } else if (command[0] == "create") {
                Chat* newChatRoom = create_chat(command[1]);
                for(int i = 0; chat_list[i] != NULL; i++) {
                    chat_list[i] = newChatRoom;
                }
            } else if (command[0] == "join") {
                join_chat(command[1], command[2], chat_list);
            } else if (command[0] == "leave") {

            } else if (command[0] == "message") {

            } else if (command[0] == "attendances") {

            } 
            

            // Respond to the client
            snprintf(sendBuff, sizeof(sendBuff), "Server received: %s", line);
            if (send(newSd, sendBuff, strlen(sendBuff), 0) < 0) {
                perror("Cannot send data");
                close(newSd);
                break;
            }

            // Clear the line buffer for the next message
            memset(line, 0x0, MAX_MSG);
        }
    }

    // Close the server socket
    close(sd);
    return 0;
}
