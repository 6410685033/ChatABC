// file.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

#define NAME_SIZE 20
#define MAX_ATTENDEES 10
#define ATTENDEE_NAME_SIZE 1024
#define MESSAGE_SIZE 1024
#define BUFFER_SIZE 1024
#define MAX_CHAT 5

typedef struct {
    char file_name[NAME_SIZE];
    char attendances[MAX_ATTENDEES][ATTENDEE_NAME_SIZE];
    int num_attendees;
    char* content;
} File;

File* chat_list[MAX_CHAT];
int chat_list_length = 0;

void remove_newline(char* str) {
    char* newline_pos = strchr(str, '\n');
    if (newline_pos != NULL) {
        *newline_pos = '\0';
    }
}

char* update_message(File *file, char* sender, char* message) {
    // Remove newline character from file->attendances[0] and sender
    remove_newline(file->attendances[0]);
    remove_newline(sender);

    printf("%s compare to %s\n", file->attendances[0], sender);
    if (strcmp(file->attendances[0], sender) != 0) {
        // Return a static string, not to be freed
        return "Wait! you not the editor.";
    }

    char *response = (char *)malloc(MESSAGE_SIZE);
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    response[0] = '\0';

    strcat(response, "message ");
    strcat(response, message);

    // Update the file content with the new message
    if (file->content != NULL) {
        free(file->content); // Free the old content
    }
    file->content = strdup(response);
    
    return response;
}

void create_file(File *file, const char *file_name) {
    if (file == NULL) {
        fprintf(stderr, "Error: file pointer is NULL.\n");
        return;
    }
    if (file_name == NULL) {
        fprintf(stderr, "Warning: file_name is NULL. Initializing with default name.\n");
        strcpy(file->file_name, "Default File");
    } else {
        strncpy(file->file_name, file_name, NAME_SIZE - 1);
        file->file_name[NAME_SIZE - 1] = '\0';
    }
    file->num_attendees = 0;
}

void print_file_info(const File *file) {
    printf("File Name: %s\n", file->file_name);
    printf("Owner: %s\n", file->attendances[0]);
    printf("Attendees (%d):\n", file->num_attendees);
    for (int i = 0; i < file->num_attendees; i++) {
        printf("%d. %s\n", i + 1, file->attendances[i]);
    }
}

char* show_attendees(const File *file) {
    char *response = (char *)malloc(file->num_attendees * ATTENDEE_NAME_SIZE);
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    response[0] = '\0';

    strcat(response, "attendances ");
    for (int i = 0; i < file->num_attendees; i++) {
        char temp_name[ATTENDEE_NAME_SIZE];
        strncpy(temp_name, file->attendances[i], ATTENDEE_NAME_SIZE - 1);
        temp_name[ATTENDEE_NAME_SIZE - 1] = '\0';

        // Remove newline characters from each attendee name
        remove_newline(temp_name);

        strcat(response, temp_name);
        if (i < file->num_attendees - 1) {
            strcat(response, " ");
        }
    }

    strcat(response, "\n");
    return response;
}

int join_file(File *file, const char *attendee) {
    if (file->num_attendees >= MAX_ATTENDEES) {
        printf("File is full. Can't add more attendees.\n");
        return -1;
    }

    strncpy(file->attendances[file->num_attendees], attendee, ATTENDEE_NAME_SIZE - 1);
    file->attendances[file->num_attendees][ATTENDEE_NAME_SIZE - 1] = '\0';
    // Remove newline characters from the new attendee name
    remove_newline(file->attendances[file->num_attendees]);
    file->num_attendees++;

    return 0;
}

int leave_file(File *file, char *attendee) {
    int index = -1;

    for (int i = 0; i < file->num_attendees; i++) {
        // Remove newline characters from attendee name before comparison
        remove_newline(file->attendances[i]);
        remove_newline(attendee);
        if (strcmp(file->attendances[i], attendee) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Attendee '%s' not found in the file.\n", attendee);
        return -1;
    }

    printf("Attendee '%s' found at index %d.\n", attendee, index);
    for (int i = index; i < file->num_attendees - 1; i++) {
        strncpy(file->attendances[i], file->attendances[i + 1], ATTENDEE_NAME_SIZE - 1);
        file->attendances[i][ATTENDEE_NAME_SIZE - 1] = '\0';
    }

    file->attendances[file->num_attendees - 1][0] = '\0';
    file->num_attendees--;

    return 0;
}

// Function to replace ";;;" with "\n" in a message
void replace_semicolons_with_newlines(char* message) {
    char* src = message;
    char* dest = message;
    while (*src) {
        if (src[0] == ';' && src[1] == ';' && src[2] == ';') {
            *dest++ = '\n';
            src += 3;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

// Function to initialize chat rooms from .txt files in a directory
void initialize_chat_rooms_from_directory(const char *directory) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(directory)) == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".txt") == 0) {
                // Remove .txt extension to get the chat room name
                char chat_room_name[NAME_SIZE];
                strncpy(chat_room_name, entry->d_name, ext - entry->d_name);
                chat_room_name[ext - entry->d_name] = '\0';

                if (chat_list_length < MAX_CHAT) {
                    File *new_chat = (File *)malloc(sizeof(File));
                    if (new_chat == NULL) {
                        perror("Failed to allocate memory for chat room");
                        continue;
                    }

                    create_file(new_chat, chat_room_name);

                    // Read the content of the file and set it to new_chat->content
                    char full_filename[BUFFER_SIZE];
                    snprintf(full_filename, BUFFER_SIZE, "%s/%s", directory, entry->d_name);
                    FILE *file = fopen(full_filename, "r");
                    if (file == NULL) {
                        perror("Failed to open chat room file");
                        free(new_chat);
                        continue;
                    }

                    fseek(file, 0, SEEK_END);
                    long file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    new_chat->content = (char *)malloc(file_size + 1);
                    if (new_chat->content == NULL) {
                        perror("Failed to allocate memory for chat content");
                        fclose(file);
                        free(new_chat);
                        continue;
                    }

                    fread(new_chat->content, 1, file_size, file);
                    new_chat->content[file_size] = '\0';
                    fclose(file);

                    // Add the new chat room to the chat list
                    chat_list[chat_list_length++] = new_chat;
                } else {
                    printf("Maximum number of chat rooms reached.\n");
                    break;
                }
            }
        }
    }
    closedir(dir);
}

