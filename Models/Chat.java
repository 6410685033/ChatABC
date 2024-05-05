package Models;

import java.util.ArrayList;
import java.util.List;

public class Chat {
    private static final int MAX_ATTENDANCE = 5;
    private String chatName;
    private List<String> attendances;

    public Chat(String chatName) {
        this.chatName = chatName;
        this.attendances = new ArrayList<>(); // Initialize an empty ArrayList
    }

    public int num_attendances() {
        return this.attendances.size();
    }

    // Method to add a member to the attendance list
    public boolean join(String member) {
        if (attendances.size() < MAX_ATTENDANCE) {
            if (!attendances.contains(member)) {
                attendances.add(member);
                System.out.println(member + " joined the chat.");
                return true;
            } else {
                System.out.println(member + " is already in the chat.");
                return false;
            }
        } else {
            System.out.println("Cannot add " + member + ". Chat room is full.");
            return false;
        }
    }

    // Method to remove a member from the attendance list
    public boolean leave(String member) {
        if (attendances.remove(member)) {
            System.out.println(member + " has left the chat.");
            return true;
        } else {
            System.out.println(member + " is not in the chat.");
            return false;
        }
    }

    // Optional: Getter to print out or use the chat name
    public String getChatName() {
        return chatName;
    }

    // Optional: Print the list of current attendees
    public void printAttendees() {
        System.out.print("Current members in chat '" + chatName + "': ");
        for (int i = 0; i < attendances.size(); i++) {
            System.out.print(attendances.get(i) + (i < attendances.size() - 1 ? ", " : "\n"));
        }
    }
}
