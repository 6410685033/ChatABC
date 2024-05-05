#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 7777
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

#define NAME_SIZE 20
#define MAX_ATTENDEES 10
#define ATTENDEE_NAME_SIZE 15
typedef struct {
    int socket;
    char room[BUFFER_SIZE];
    char username[BUFFER_SIZE];
} Client;

// Define the Chat struct
typedef struct {
    char chat_name[NAME_SIZE];
    char attendances[MAX_ATTENDEES][ATTENDEE_NAME_SIZE];
    int num_attendees;
} Chat;

void create_chat(Chat *chat, const char *chat_name) {
    strncpy(chat->chat_name, chat_name, NAME_SIZE - 1); // set the chat name
    chat->chat_name[NAME_SIZE - 1] = '\0'; // null-terminate the string
    chat->num_attendees = 0; // set the number of attendees to 0
}

int join_chat(Chat *chat, const char *attendee) {
    // Check if the chat room is full
    if (chat->num_attendees >= MAX_ATTENDEES) {
        printf("Chat room is full. Can't add more attendees.\n");
        return -1;
    }

    strncpy(chat->attendances[chat->num_attendees], attendee, ATTENDEE_NAME_SIZE - 1); // add the attendee
    chat->attendances[chat->num_attendees][ATTENDEE_NAME_SIZE - 1] = '\0'; // null-terminate the string
    chat->num_attendees++; // increase the number of attendees

    return 0;
}

int leave_chat(Chat *chat, const char *attendee) {
    int index = -1;

    // Find the attendee index in the array
    for (int i = 0; i < chat->num_attendees; i++) {
        if (strcmp(chat->attendances[i], attendee) == 0) {
            index = i;
            break;
        }
    }

    // If attendee not found, return error
    if (index == -1) {
        printf("Attendee '%s' not found in the chat.\n", attendee);
        return -1;
    }

    // Shift remaining attendees to remove the target attendee
    for (int i = index; i < chat->num_attendees - 1; i++) {
        strncpy(chat->attendances[i], chat->attendances[i + 1], ATTENDEE_NAME_SIZE - 1);
        chat->attendances[i][ATTENDEE_NAME_SIZE - 1] = '\0';
    }

    // Clear the last element as it is now empty
    chat->attendances[chat->num_attendees - 1][0] = '\0';
    chat->num_attendees--;

    return 0; // Success
}

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    Client *client = (Client *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = read(client->socket, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        pthread_mutex_lock(&client_mutex);
        for (int i = 0; i < client_count; i++) {
            if (clients[i].socket != client->socket && strcmp(clients[i].room, client->room) == 0) {
                write(clients[i].socket, buffer, bytes_read);
            }
        }
        pthread_mutex_unlock(&client_mutex);
    }

    // Client disconnect
    close(client->socket);
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == client->socket) {
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);
    free(client);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t tid;

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket failed");
        exit(1);
    }

    // Set server address parameters
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
            perror("Accept failed");
            continue;
        }

        // Create a new client struct
        Client *new_client = malloc(sizeof(Client));
        new_client->socket = client_socket;

        // Read username and room from the client
        char initial_message[BUFFER_SIZE];
        int initial_len = read(client_socket, initial_message, BUFFER_SIZE - 1);
        if (initial_len > 0) {
            initial_message[initial_len] = '\0';
            char *colon = strchr(initial_message, ':');
            if (colon != NULL) {
                *colon = '\0';
                strcpy(new_client->username, initial_message);
                strcpy(new_client->room, colon + 1);
            }
        }

        // Add the new client to the list
        pthread_mutex_lock(&client_mutex);
        clients[client_count++] = *new_client;
        pthread_mutex_unlock(&client_mutex);

        // Create a new thread for the client
        pthread_create(&tid, NULL, &handle_client, new_client);
        pthread_detach(tid);
    }

    close(server_socket);
    return 0;
}
