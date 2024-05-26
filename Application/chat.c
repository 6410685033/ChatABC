// chat.c
#include <stdio.h>
#include <string.h>

#define NAME_SIZE 20
#define MAX_ATTENDEES 10
#define ATTENDEE_NAME_SIZE 1024
#define MAX_LINE 10

typedef struct {
    char chat_name[NAME_SIZE];
    char attendances[MAX_ATTENDEES][ATTENDEE_NAME_SIZE];
    int num_attendees;
    char* file[MAX_LINE];
} Chat;

void update_file(Chat *chat, int line_index, char* newMessage) {
    chat->file[line_index] = newMessage;
}

void create_chat(Chat *chat, const char *chat_name) {
    if (chat == NULL) {
        fprintf(stderr, "Error: chat pointer is NULL.\n");
        return;  // Optionally, you could change the function return type to int and return an error code here
    }
    if (chat_name == NULL) {
        fprintf(stderr, "Warning: chat_name is NULL. Initializing with default name.\n");
        strcpy(chat->chat_name, "Default Chat");  // Use a default name if none provided
    } else {
        strncpy(chat->chat_name, chat_name, NAME_SIZE - 1);
        chat->chat_name[NAME_SIZE - 1] = '\0';  // Ensures null termination
    }
    chat->num_attendees = 0;
}


char* join_chat(Chat *chat, char* attendee) {
    if (chat->num_attendees >= MAX_ATTENDEES) {
        printf("Chat room is full. Can't add more attendees.\n");
        return "join full";
    }

    strncpy(chat->attendances[chat->num_attendees], attendee, ATTENDEE_NAME_SIZE - 1);
    chat->attendances[chat->num_attendees][ATTENDEE_NAME_SIZE - 1] = '\0';
    chat->num_attendees++;

    return "join success";
}

char* leave_chat(Chat *chat, const char *attendee) {
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
        return "leave fail";
    }

    // Shift remaining attendees to remove the target attendee
    for (int i = index; i < chat->num_attendees - 1; i++) {
        strncpy(chat->attendances[i], chat->attendances[i + 1], ATTENDEE_NAME_SIZE - 1);
        chat->attendances[i][ATTENDEE_NAME_SIZE - 1] = '\0';
    }

    // Clear the last element as it is now empty
    chat->attendances[chat->num_attendees - 1][0] = '\0';
    chat->num_attendees--;

    return "leave success"; // Success
}

void print_chat_info(const Chat *chat) {
    printf("Chat Name: %s\n", chat->chat_name);
    printf("Attendees (%d):\n", chat->num_attendees);
    for (int i = 0; i < chat->num_attendees; i++) {
        printf("%d. %s\n", i + 1, chat->attendances[i]);
    }
}

char* show_attendees(Chat* chat) {
    char* response = (char*)malloc(ATTENDEE_NAME_SIZE * chat->num_attendees);
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    response[0] = '\0';
    strcpy(response, "attendances ");

    char attendee_name[ATTENDEE_NAME_SIZE];

    for (int i = 0; i < chat->num_attendees; i++) {
        char* attendee_name = chat->attendances[i];
        char* newline_pos;
        while ((newline_pos = strchr(attendee_name, '\n')) != NULL) {
            *newline_pos = '\0';
        }

        strcat(response, attendee_name);
        
        if (i < chat->num_attendees - 1) {
            strcat(response, " ");
        }
    }

    strcat(response, "\n");
    return response;
}