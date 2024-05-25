// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "chat.c"

#define PORT 7777
#define BUFFER_SIZE 1024
#define MAX_MSG 100
#define MAX_TOKENS 4
#define MAX_CHAT 10
#define MAX_CLIENTS 100

// Client struct definition
typedef struct {
    int socket;
    char room[NAME_SIZE];
    char username[BUFFER_SIZE];
} Client;

Client clients[MAX_CLIENTS];  // Array of clients
int num_clients = 0;  // Track the number of connected clients
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Chat room list and its length
Chat* chat_list[MAX_CHAT] = {0};
int chat_list_length = 0;

void broadcast_message(const char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (send(clients[i].socket, message, strlen(message), 0) < 0) {
            perror("Broadcast failed");
            continue;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
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
    static char *tokens[MAX_TOKENS];
    int n = 0;
    char *token = strtok(line, " ");

    while (token != NULL && n < MAX_TOKENS) {
        tokens[n++] = token;
        token = strtok(NULL, " ");
    }

    return tokens;
}

char* response_list_room(Chat* chat_list[], int chat_list_length) {
    char* response = (char*)malloc(BUFFER_SIZE * chat_list_length);
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    response[0] = '\0';
    strcpy(response, "list_rooms ");

    char room_info[BUFFER_SIZE];

    for (int i = 0; i < chat_list_length; i++) {
        char* chat_name = chat_list[i]->chat_name;
        char* newline_pos;
        while ((newline_pos = strchr(chat_name, '\n')) != NULL) {
            *newline_pos = '\0';
        }
        
        snprintf(room_info, sizeof(room_info), "%s", chat_name);
        strcat(response, room_info);
        
        if (i < chat_list_length - 1) {
            strcat(response, " ");
        }
    }

    strcat(response, "\n");
    return response;
}

void* handle_client(void* arg) {
    int newSd = *((int*)arg);
    free(arg);
    char line[MAX_MSG];

    printf("Client handler thread started for socket %d\n", newSd);

    memset(line, 0x0, MAX_MSG);

    while (read_line(newSd, line) != -1) {
        printf("Received from client: %s", line);
        char** command = decode(line);
        size_t len = strlen(command[0]);
        
        if (command[0][len - 1] == '\n') {
            command[0][len - 1] = '\0';
        }

        printf("Command: %s\n", command[0]);

        if (strcmp(command[0], "login") == 0) {
            printf("Login\n");
            // Implement login logic
        } else if (strcmp(command[0], "logout") == 0) {
            printf("Logout\n");
            // Implement logout logic
        } else if (strcmp(command[0], "create") == 0) {
            printf("Creating chat room...\n");
            if (command[1] == NULL) {
                printf("Error: Chat name must be provided.\n");
                continue;
            }

            int i = 0;
            for (; i < MAX_CHAT && chat_list[i] != NULL; i++);

            if (i == MAX_CHAT) {
                printf("Error: Maximum number of chats reached.\n");
                continue;
            }

            chat_list[i] = malloc(sizeof(Chat));
            if (chat_list[i] == NULL) {
                perror("Failed to allocate memory for new chat room");
                continue;
            }

            create_chat(chat_list[i], command[1]);
            chat_list_length++;

            char broadcast_msg[BUFFER_SIZE];
            snprintf(broadcast_msg, sizeof(broadcast_msg), "New chat room created: %s\n", command[1]);
            broadcast_message(broadcast_msg);
            printf("Chat room created: %s\n", command[1]);
            printf("Room created: %s\n", chat_list[i]->chat_name);
        } else if (strcmp(command[0], "join") == 0) {
            printf("Join chat room...\n");
            // Implement join chat room logic
        } else if (strcmp(command[0], "leave") == 0) {
            printf("Leave chat room...\n");
            // Implement leave chat room logic
        } else if (strcmp(command[0], "message") == 0) {
            printf("Message chat room...\n");
            // Implement message sending logic
        } else if (strcmp(command[0], "attendances") == 0) {
            printf("Attendances chat room...\n");
            // Implement attendance checking logic
        } else if (strcmp(command[0], "list_rooms") == 0) {
            printf("Listing chat rooms...\n");

            char* response = response_list_room(chat_list, chat_list_length);

            printf("Send response...\n");
            printf("%s\n", response);

            if (send(newSd, response, strlen(response), 0) < 0) {
                perror("Failed to send chat room list");
            }

            free(response);
        } else {
            printf("Invalid command\n");
        }

        memset(line, 0x0, MAX_MSG);
    }

    close(newSd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int sd;
    struct sockaddr_in servAddr, cliAddr;
    socklen_t cliLen;

    // Chat room list and its length are now global

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
        int *newSd = malloc(sizeof(int));
        *newSd = accept(sd, (struct sockaddr *)&cliAddr, &cliLen);
        if (*newSd < 0) {
            perror("Cannot accept connection");
            free(newSd);
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (num_clients < MAX_CLIENTS) {
            clients[num_clients].socket = *newSd;
            strncpy(clients[num_clients].room, "", NAME_SIZE);
            strncpy(clients[num_clients].username, "", BUFFER_SIZE);
            num_clients++;
        } else {
            printf("Maximum client limit reached.\n");
            close(*newSd);
            free(newSd);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        pthread_mutex_unlock(&clients_mutex);

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, newSd) != 0) {
            perror("Failed to create thread");
        }
    }

    close(sd);
    return 0;
}
