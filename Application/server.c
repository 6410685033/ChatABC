// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "chat.c"

#define PORT 7777
#define BUFFER_SIZE 1024
#define MAX_MSG 100
#define MAX_TOKENS 4
#define MAX_CHAT 4
#define MAX_CLIENTS 100


// Client struct definition
typedef struct {
    int socket;
    char room[NAME_SIZE];
    char username[BUFFER_SIZE];
} Client;

Client clients[MAX_CLIENTS];  // Array of clients
int num_clients = 0;  // Track the number of connected clients

void broadcast_message(const char *message) {
    for (int i = 0; i < num_clients; i++) {
        if (send(clients[i].socket, message, strlen(message), 0) < 0) {
            perror("Broadcast failed");
            continue;
        }
    }
}


int read_line(int newSd, char *line_to_return) {
    static int rcv_ptr = 0;
    static char rcv_msg[MAX_MSG] = {0};
    static int n = 0;
    int offset = 0;

    while (1) {
        if (rcv_ptr == 0) {
            memset(rcv_msg, 0x0, MAX_MSG);  // Clear the buffer
            n = recv(newSd, rcv_msg, MAX_MSG, 0);
            if (n < 0) {
                perror("Cannot receive data");
                return -1;
            } else if (n == 0) {
                printf("Connection closed by client\n");
                close(newSd);
                return -1;
            }
        }

        while (rcv_ptr < n && rcv_msg[rcv_ptr] != '\n') {
            line_to_return[offset++] = rcv_msg[rcv_ptr++];
        }

        if (rcv_ptr < n && rcv_msg[rcv_ptr] == '\n') {
            line_to_return[offset++] = '\n';
            rcv_ptr++;
            return offset;
        }

        if (rcv_ptr == n) {
            rcv_ptr = 0;
        }
    }
}

char** decode(char* line) {
    static char *tokens[MAX_TOKENS];  // static storage for return
    int n = 0;
    char *token = strtok(line, " ");  // Use " " instead of ' '

    while (token != NULL && n < MAX_TOKENS) {
        tokens[n++] = token;
        token = strtok(NULL, " ");
    }

    return tokens;
}

int main(int argc, char *argv[]) {
    int sd, newSd;
    socklen_t cliLen;
    struct sockaddr_in servAddr, cliAddr;
    char line[MAX_MSG];
    char sendBuff[BUFFER_SIZE];

    Chat* chat_list[MAX_CHAT] = {0};  // Initialize to NULL

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("Cannot open socket");
        return 1;
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("Cannot bind port");
        close(sd);
        return 1;
    }

    listen(sd, 5);
    printf("Server waiting for data on port TCP %u\n", PORT);

    while (1) {
        cliLen = sizeof(cliAddr);
        newSd = accept(sd, (struct sockaddr *)&cliAddr, &cliLen);
        if (newSd < 0) {
            perror("Cannot accept connection");
            continue;
        }

        if (num_clients < MAX_CLIENTS) {
        clients[num_clients].socket = newSd;
        strncpy(clients[num_clients].room, "", NAME_SIZE);  // Initialize with no room
        strncpy(clients[num_clients].username, "", BUFFER_SIZE);  // No username initially
        num_clients++;
        } else {
            printf("Maximum client limit reached.\n");
            close(newSd);  // Close the new socket as it can't be handled
            continue;
        }

        printf("Client connected from %s:%d\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));
        memset(line, 0x0, MAX_MSG);

        while (read_line(newSd, line) != -1) {
            printf("Received from client: %s", line);
            char** command = decode(line);
            size_t len = strlen(command[0]);
            
            if (command[0][len - 1] == '\n') {
                command[0][len - 1] = '\0';  // Remove the newline character if present
            }

            printf("Command: %s\n", command[0]);

            // Handle commands
            if (strcmp(command[0], "login") == 0) {
                printf("Login\n");
                // Implement login logic
            } else if (strcmp(command[0], "logout") == 0) {
                // Implement logout logic
                printf("Logout\n");
            } else if (strcmp(command[0], "create") == 0) {
                printf("Creating chat room...\n");
                if (command[1] == NULL) {
                    printf("Error: Chat name must be provided.\n");
                    continue;
                }
                
                // Find the next available slot in the chat list
                int i = 0;
                for (; i < MAX_CHAT && chat_list[i] != NULL; i++);

                if (i == MAX_CHAT) {
                    printf("Error: Maximum number of chats reached.\n");
                    continue;
                }

                // Allocate memory for the new chat room
                chat_list[i] = malloc(sizeof(Chat));
                if (chat_list[i] == NULL) {
                    perror("Failed to allocate memory for new chat room");
                    continue;
                }

                // Initialize the chat room with the provided name
                create_chat(chat_list[i], command[1]);

                // Broadcast message to all connected clients that a new chat room has been created
                char broadcast_msg[BUFFER_SIZE];
                snprintf(broadcast_msg, sizeof(broadcast_msg), "New chat room created: %s\n", command[1]);
                broadcast_message(broadcast_msg);
                printf("Chat room created: %s\n", command[1]);
                printf("Room created: %s\n",chat_list[i]->chat_name);
            }
            else if (strcmp(command[0], "join") == 0) {
                // Join chat logic, ensure room exists in chat_list
                printf("join chat rooms...\n");
            } else if (strcmp(command[0], "leave") == 0) {
                // Leave chat logic
                printf("leave chat rooms...\n");
            } else if (strcmp(command[0], "message") == 0) {
                // Message sending logic
                printf("message chat rooms...\n");
            } else if (strcmp(command[0], "attendances") == 0) {
                // Attendance checking logic
                printf("attendances chat rooms...\n");
            } else if (strcmp(command[0], "list_rooms") == 0) {
                printf("Listing chat rooms...\n");

                char room_info[BUFFER_SIZE];
                char response[BUFFER_SIZE] = "list_rooms"; // Start with a header for the response
                int response_len = strlen(response);

                // Loop through all chat rooms and collect their names
                for (int i = 0; i < MAX_CHAT; i++) {
                    if (chat_list[i] != NULL) { // Ensure the chat room slot is not empty
                        snprintf(room_info, sizeof(room_info), " %s", chat_list[i]->chat_name);
                        strcat(response + response_len, room_info);
                        response_len += strlen(room_info); // Update response length
                    }
                }

                printf("Send response...\n");
                printf("%s\n", response);

                // Send the response to the client that requested the room list
                if (send(newSd, response, strlen(response), 0) < 0) {
                    perror("Failed to send chat room list");
                }

            }
            else {
                printf("Invalid command\n");
            }

            memset(line, 0x0, MAX_MSG);  // Reset line buffer
        }
    }

    close(sd);
    return 0;
}