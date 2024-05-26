import javax.swing.*;
import java.awt.*;
import java.util.ArrayList;
import java.util.List;

public class ChatRoomPanel extends JPanel {
    private static JTextArea chatArea;
    private static List<String> currentParticipants = new ArrayList<>();

    public ChatRoomPanel(ChatClientGUI chatClientGUI, String roomName) {
        setLayout(new BorderLayout());
        chatArea = new JTextArea();
        chatArea.setEditable(false);
        JTextField messageField = new JTextField();
        JButton sendButton = new JButton("Send");
        JButton leaveRoomButton = new JButton("Leave Room");
        JButton showParticipantsButton = new JButton("Show Participants");

        sendButton.addActionListener(e -> {
            String message = messageField.getText().trim();
            if (!message.isEmpty()) {
                chatClientGUI.getConnectionManager().getWriter().println("message " + roomName + " " + chatClientGUI.getUsername() + " " + message);
                messageField.setText("");
            }
        });

        leaveRoomButton.addActionListener(e -> {
            chatClientGUI.getConnectionManager().getWriter().println("leave " + roomName + " " + chatClientGUI.getUsername());
            chatClientGUI.showChatListPage();
        });

        showParticipantsButton.addActionListener(e -> {
            chatClientGUI.getConnectionManager().getWriter().println("attendances " + roomName);
            JOptionPane.showMessageDialog(null, "Participants: " + String.join(", ", currentParticipants));
        });

        JPanel topPanel = new JPanel(new BorderLayout());
        topPanel.add(leaveRoomButton, BorderLayout.WEST);
        topPanel.add(showParticipantsButton, BorderLayout.EAST);
        topPanel.add(new JLabel(roomName, JLabel.CENTER), BorderLayout.CENTER);

        JPanel inputPanel = new JPanel(new BorderLayout());
        inputPanel.add(messageField, BorderLayout.CENTER);
        inputPanel.add(sendButton, BorderLayout.EAST);

        add(new JScrollPane(chatArea), BorderLayout.CENTER);
        add(topPanel, BorderLayout.NORTH);
        add(inputPanel, BorderLayout.SOUTH);
    }

    public static void updateAttendances(String attendancesResponse) {
        String[] parts = attendancesResponse.split(" ");
        currentParticipants.clear();
        for (int i = 1; i < parts.length; i++) {
            currentParticipants.add(parts[i]);
        }
    }

    public static void updateMessage(String messageResponse) {
        String[] parts = messageResponse.split(" ", 3);
        if (parts.length > 2) {
            chatArea.append(parts[1] + ": " + parts[2] + "\n");
        }
    }
}
