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

void create_chat(Chat *chat, const char *chat_name) {
    strncpy(chat->chat_name, chat_name, NAME_SIZE - 1);
    chat->chat_name[NAME_SIZE - 1] = '\0';
    chat->num_attendees = 0;
}

int join_chat(Chat *chat, const char *attendee) {
    if (chat->num_attendees >= MAX_ATTENDEES) {
        printf("Chat room is full. Can't add more attendees.\n");
        return -1;
    }

    strncpy(chat->attendances[chat->num_attendees], attendee, ATTENDEE_NAME_SIZE - 1);
    chat->attendances[chat->num_attendees][ATTENDEE_NAME_SIZE - 1] = '\0';
    chat->num_attendees++;

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

void print_chat_info(const Chat *chat) {
    printf("Chat Name: %s\n", chat->chat_name);
    printf("Attendees (%d):\n", chat->num_attendees);
    for (int i = 0; i < chat->num_attendees; i++) {
        printf("%d. %s\n", i + 1, chat->attendances[i]);
    }
}
