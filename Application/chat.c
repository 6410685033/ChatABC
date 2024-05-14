#include <stdio.h>
#include <string.h>

#define NAME_SIZE 20
#define MAX_ATTENDEES 10
#define ATTENDEE_NAME_SIZE 15

typedef struct {
    char chat_name[NAME_SIZE];
    char attendances[MAX_ATTENDEES][ATTENDEE_NAME_SIZE];
    int num_attendees;
} Chat;

Chat* create_chat(const char *chat_name) {
    // Allocate memory for a new Chat structure
    Chat* new_chat = (Chat*) malloc(sizeof(Chat));
    if (new_chat == NULL) {
        perror("Failed to allocate memory for new chat");
        return NULL;  // Return NULL if memory allocation fails
    }

    // Safely copy the chat name into the structure
    strncpy(new_chat->chat_name, chat_name, NAME_SIZE - 1);
    new_chat->chat_name[NAME_SIZE - 1] = '\0';  // Ensure null-termination

    // Initialize the number of attendees
    new_chat->num_attendees = 0;

    // Return the newly created chat structure
    return new_chat;
}

// Function to find a room by name in a NULL-terminated array of Chat pointers
Chat* find_room(Chat* chat_list[], const char* room_name) {
    for (int i = 0; chat_list[i] != NULL; i++) {  // Loop through each room until NULL is encountered
        if (strcmp(room_name, chat_list[i]->chat_name) == 0) {  // Compare room names
            return chat_list[i];  // Return the matching room
        }
    }
    return NULL;  // Return NULL if no room matches
}

void join_chat(const char* room_name, const char* username, Chat* chat_list[]) {
    Chat* chat = find_room(chat_list, room_name);  // Find the correct chat room by name
    if (chat != NULL && chat->num_attendees < MAX_ATTENDEES) {
        // Allocate memory for new username and add it to the list
        chat->attendances[chat->num_attendees] = malloc(strlen(username) + 1);
        if (chat->attendances[chat->num_attendees] != NULL) {
            strcpy(chat->attendances[chat->num_attendees], username);
            chat->num_attendees++;
        }
    } else {
        if (chat == NULL) {
            printf("Chat room '%s' not found.\n", room_name);
        } else {
            printf("Chat room '%s' is full.\n", room_name);
        }
    }
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

void print_chat_info(const Chat *chat) {
    printf("Chat Name: %s\n", chat->chat_name);
    printf("Attendees (%d):\n", chat->num_attendees);
    for (int i = 0; i < chat->num_attendees; i++) {
        printf("%d. %s\n", i + 1, chat->attendances[i]);
    }
}
