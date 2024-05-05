#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Replace with the appropriate server IP
#define PORT 7777
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    char name[BUFFER_SIZE];

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert IP addresses from text to binary
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Enter your name: ");
    fgets(name, BUFFER_SIZE, stdin);
    name[strcspn(name, "\n")] = '\0'; // Remove the newline character

    // Send the name to the server
    send(sock, name, strlen(name), 0);

    // Read and send messages
    while (1) {
        printf("Enter message to send (or 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove the newline character

        if (strcmp(buffer, "exit") == 0) {
            printf("Closing the connection.\n");
            break;
        }

        // Send message to server
        send(sock, buffer, strlen(buffer), 0);

        // Receive server response
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Server response: %s\n", buffer);
        }
    }

    // Close the socket
    close(sock);
    return 0;
}
