// server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "file.c"

#define PORT 7777
#define BUFFER_SIZE 1024
#define MAX_MSG 1024
#define MAX_TOKENS 4
#define MAX_CHAT 5
#define MAX_CLIENTS 5

typedef struct {
    int socket;
    File* file;
    char username[BUFFER_SIZE];
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

File* find_chat(File* chat_list[], int chat_list_length, const char* chat_name) {
    char clean_chat_name[BUFFER_SIZE];
    strncpy(clean_chat_name, chat_name, BUFFER_SIZE - 1);
    clean_chat_name[BUFFER_SIZE - 1] = '\0';  // Ensure null-termination

    // Remove the newline character, if present
    char* newline_pos = strchr(clean_chat_name, '\n');
    if (newline_pos != NULL) {
        *newline_pos = '\0';
    }

    for (int i = 0; i < chat_list_length; i++) {
        printf("%s compare to %s\n", chat_list[i]->file_name, clean_chat_name);
        if (strcmp(chat_list[i]->file_name, clean_chat_name) == 0) {
            return chat_list[i];
        }
    }
    return NULL; 
}

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

void broadcast_attendances(const char *message, File *file) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].file != NULL && strcmp(clients[i].file->file_name, file->file_name) == 0) {
            if (send(clients[i].socket, message, strlen(message), 0) < 0) {
                perror("Broadcast failed");
                continue;
            }
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

    while (token != NULL && n < MAX_TOKENS - 1) { // Reserve the last token for potential concatenation
        tokens[n++] = token;
        token = strtok(NULL, " ");
    }

    if (token != NULL) {
        // Concatenate the remaining parts with spaces
        size_t remaining_length = strlen(token) + 1; // Include space for the null terminator
        char *remaining = (char *)malloc(remaining_length * sizeof(char));
        strcpy(remaining, token);

        while ((token = strtok(NULL, " ")) != NULL) {
            remaining_length += strlen(token) + 1; // +1 for the space
            remaining = (char *)realloc(remaining, remaining_length * sizeof(char));
            strcat(remaining, " ");
            strcat(remaining, token);
        }
        tokens[n++] = remaining; // Add the concatenated string as the last token
    }

    tokens[n] = NULL; // Terminate the list with a NULL pointer
    return tokens;
}

char* response_list_room(File* chat_list[], int chat_list_length) {
    char* response = (char*)malloc(BUFFER_SIZE);
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    response[0] = '\0';
    strcpy(response, "list_rooms ");

    char room_info[BUFFER_SIZE];

    for (int i = 0; i < chat_list_length; i++) {
        char* file_name = chat_list[i]->file_name;
        char* newline_pos;
        while ((newline_pos = strchr(file_name, '\n')) != NULL) {
            *newline_pos = '\0';
        }
        
        snprintf(room_info, sizeof(room_info), "%s", file_name);
        strcat(response, room_info);
        
        if (i < chat_list_length - 1) {
            strcat(response, " ");
        }
    }

    strcat(response, "\n");
    return response;
}

// Function to save a message to a .txt file
void save_message_to_file(File* chat, const char* message) {
    // Allocate memory for the full filename
    char full_filename[BUFFER_SIZE];
    snprintf(full_filename, BUFFER_SIZE, "%s.txt", chat->file_name);

    // Open the file in write mode, it will replace the file if it exists.
    FILE* file = fopen(full_filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", full_filename);
        return;
    }

    // Remove "message " prefix and replace ";;;" with "\n"
    const char* message_start = strchr(message, ' ') + 1;
    char formatted_message[BUFFER_SIZE];
    strncpy(formatted_message, message_start, BUFFER_SIZE - 1);
    formatted_message[BUFFER_SIZE - 1] = '\0';
    replace_semicolons_with_newlines(formatted_message);

    // Write the formatted message to the file
    fprintf(file, "%s\n", formatted_message);

    // Close the file
    fclose(file);
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
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < num_clients; i++) {
                if (clients[i].socket == newSd) {
                    strncpy(clients[i].username, command[1], BUFFER_SIZE - 1);
                    clients[i].username[BUFFER_SIZE - 1] = '\0';
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            printf("%s %s\n", command[0], command[1]);
        } else if (strcmp(command[0], "logout") == 0) {
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < num_clients; i++) {
                if (clients[i].socket == newSd) {
                    clients[i].file = NULL;
                    clients[i].username[0] = '\0';
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            printf("%s %s\n", command[0], command[1]);
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

            chat_list[i] = malloc(sizeof(File));
            if (chat_list[i] == NULL) {
                perror("Failed to allocate memory for new chat room");
                continue;
            }

            create_file(chat_list[i], command[1]);
            chat_list_length++;

            char* response = response_list_room(chat_list, chat_list_length);

            printf("Send response...\n");
            printf("%s\n", response);

            broadcast_message(response);

            free(response);

        } else if (strcmp(command[0], "join") == 0) {
            printf("Join chat room...\n");

            File* current_chat = find_chat(chat_list, chat_list_length, command[1]);
            if (current_chat != NULL) {
                int result_join = join_file(current_chat, command[2]);
                if (result_join == 0) {
                    pthread_mutex_lock(&clients_mutex);
                    for (int i = 0; i < num_clients; i++) {
                        if (strcmp(clients[i].username, command[2]) == 0) {
                            clients[i].file = current_chat;
                        }
                    }
                    pthread_mutex_unlock(&clients_mutex);
                }
                char* response = show_attendees(current_chat);

                printf("Send response...\n");
                printf("%s\n", response);

                broadcast_attendances(response, current_chat);

                free(response);
            } else {
                printf("Chat room not found.\n");
            }

        } else if (strcmp(command[0], "leave") == 0) {
            printf("Leave chat room...\n");

            File* current_chat = find_chat(chat_list, chat_list_length, command[1]);
            if (current_chat != NULL) {
                leave_file(current_chat, command[2]);
                char* response = show_attendees(current_chat);

                printf("Send response...\n");
                printf("%s\n", response);

                print_file_info(current_chat);

                broadcast_attendances(response, current_chat);

                free(response);
            } else {
                printf("Chat room not found.\n");
            }

        } else if (strcmp(command[0], "message") == 0) {
            printf("Message chat room...\n");

            File* current_chat = find_chat(chat_list, chat_list_length, command[1]);
            if (current_chat != NULL) {
                char* response = update_message(current_chat, command[2], command[3]);
                printf("Send response...\n");
                printf("%s\n", response);
                broadcast_attendances(response, current_chat);

                // Only free if response is dynamically allocated
                if (strcmp(response, "Wait! you not the editor.") != 0) {
                    free(response);
                }
            } else {
                printf("Chat room not found.\n");
            }
        } else if (strcmp(command[0], "fetch_file") == 0) {
            printf("fetch chat room...\n");

            File* current_chat = find_chat(chat_list, chat_list_length, command[1]);
            if (current_chat != NULL) {
                char* response = current_chat->content;
                if (response != NULL && strcmp(response, "Wait! you not the editor.") != 0) {
                    printf("Send response...\n");
                    printf("%s\n", response);
                    broadcast_attendances(response, current_chat);
                } else if (response != NULL) {
                    printf("%s\n", response);
                }
            } else {
                printf("Chat room not found.\n");
            }
        } else if (strcmp(command[0], "attendances") == 0) {
            printf("Attendances chat room...\n");
            File* current_chat = find_chat(chat_list, chat_list_length, command[1]);
            if (current_chat != NULL) {
                char* response = show_attendees(current_chat);

                printf("Send response...\n");
                printf("%s\n", response);

                if (send(newSd, response, strlen(response), 0) < 0) {
                    perror("Failed to send attendances");
                }

                free(response);
            } else {
                printf("Chat room not found.\n");
            }

        } else if (strcmp(command[0], "list_rooms") == 0) {
            printf("Listing chat rooms...\n");

            char* response = response_list_room(chat_list, chat_list_length);

            printf("Send response...\n");
            printf("%s\n", response);

            if (send(newSd, response, strlen(response), 0) < 0) {
                perror("Failed to send chat room list");
            }

            free(response);

        } else if (strcmp(command[0], "editor") == 0) {
            printf("Editor of this room...\n");

            if (command[1] == NULL) {
                printf("Error: Chat name must be provided.\n");
                continue;
            }

            File* current_chat = find_chat(chat_list, chat_list_length, command[1]);
            if (current_chat != NULL) {
                char* response = (char*)malloc(BUFFER_SIZE);
                if (response == NULL) {
                    fprintf(stderr, "Memory allocation failed\n");
                    exit(1);
                }

                // Construct the response message
                snprintf(response, BUFFER_SIZE, "editor %s\n", current_chat->attendances[0]);

                char message[BUFFER_SIZE];
                snprintf(message, sizeof(message), "editor %s\n", response);

                printf("Send response...\n");
                broadcast_attendances(message, current_chat);

                free(response); // Free the allocated memory
            } else {
                printf("Chat room not found.\n");
            }

        } else if (strcmp(command[0], "save") == 0) {
            printf("Saving File...\n");

            File* current_chat = find_chat(chat_list, chat_list_length, command[1]);
            save_message_to_file(current_chat, current_chat->content); 
        } else {
            printf("Invalid command\n");
        }

        memset(line, 0x0, MAX_MSG);
    }

    close(newSd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    const char *chat_directory = argv[1];
    initialize_chat_rooms_from_directory(chat_directory);

    int sd;
    struct sockaddr_in servAddr, cliAddr;
    socklen_t cliLen;

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
            clients[num_clients].file = NULL;  // Initialize the file pointer
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
        int *arg = malloc(sizeof(*arg));
        if (arg == NULL) {
            perror("Failed to allocate memory");
            close(*newSd);
            free(newSd);
            continue;
        }
        *arg = *newSd;
        if (pthread_create(&tid, NULL, handle_client, arg) != 0) {
            perror("Failed to create thread");
            close(*newSd);
            free(newSd);
            free(arg);
        }
    }

    close(sd);
    return 0;
}
