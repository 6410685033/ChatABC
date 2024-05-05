#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* for close */
#include <errno.h>  /* for errno */

#define SUCCESS 0
#define ERROR 1

#define PORT 7777
#define MAX_MSG 1024
#define MAX_CLIENTS 5
#define NAME_SIZE 1024

/* Struct to represent each client */
typedef struct {
    int socket;
    char name[NAME_SIZE];
} Client;

int main() {
    int server_fd, new_socket, max_sd, sd, activity, i, valread;
    struct sockaddr_in address, client_addr;
    fd_set readfds;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[NAME_SIZE];
    Client clients[MAX_CLIENTS] = {0};

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        return ERROR;
    }

    // Set server address parameters
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind failed");
        close(server_fd);
        return ERROR;
    }

    // Start listening for incoming connections
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen");
        close(server_fd);
        return ERROR;
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add the server socket to the set
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add all active client sockets to the set
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].socket;

            // If valid socket descriptor, add to read set
            if (sd > 0)
                FD_SET(sd, &readfds);

            // Update max_sd for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        // Wait for activity on any of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // Handle incoming connections on the server socket
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen)) == -1) {
                perror("accept");
                continue;
            }

            printf("New connection, socket fd is %d, ip is: %s, port: %d\n",
                   new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Add the new socket to the list of client sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = new_socket;
                    strcpy(clients[i].name, "Unknown");
                    break;
                }
            }
        }

        // Check all client sockets for incoming messages
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].socket;

            if (FD_ISSET(sd, &readfds)) {
                // Check if it was a disconnection
                if ((valread = read(sd, buffer, NAME_SIZE - 1)) == 0) {
                    // Close the socket and mark as 0 for reuse
                    getpeername(sd, (struct sockaddr *)&client_addr, &addrlen);
                    printf("Host '%s' disconnected, ip %s, port %d\n",
                           clients[i].name, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    close(sd);
                    clients[i].socket = 0;
                    strcpy(clients[i].name, "Unknown");
                } else {
                    // Process the incoming message
                    buffer[valread] = '\0';

                    // If the client's name is still "Unknown," assume the first message contains the name
                    if (strcmp(clients[i].name, "Unknown") == 0) {
                        strncpy(clients[i].name, buffer, NAME_SIZE - 1);
                        clients[i].name[NAME_SIZE - 1] = '\0';
                        printf("Client connected: '%s'\n", clients[i].name);
                    } else {
                        // Otherwise, print the message received from the client
                        printf("Received from '%s': %s\n", clients[i].name, buffer);
                        send(sd, "Message received", strlen("Message received"), 0);
                    }
                }
            }
        }
    }

    return SUCCESS;
}
