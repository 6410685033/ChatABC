import javax.swing.*;
import java.awt.*;
import java.util.ArrayList;
import java.util.List;

public class RoomListPanel extends JPanel {
    private static List<String> chatRooms = new ArrayList<>();
    private JPanel chatButtonPanel = new JPanel(new GridLayout(0, 1));
    private ChatClientGUI chatClientGUI;

    public RoomListPanel(ChatClientGUI chatClientGUI) {
        this.chatClientGUI = chatClientGUI;
        setLayout(new BorderLayout());
        JScrollPane scrollPane = new JScrollPane(chatButtonPanel);
        JPanel topPanel = new JPanel(new BorderLayout());
        JButton createRoomButton = new JButton("Create New Room");
        JButton logoutButton = new JButton("Logout");
        JButton refreshButton = new JButton("Refresh");
        JLabel loggedInLabel = new JLabel("Logged in as: " + chatClientGUI.getUsername(), JLabel.RIGHT);

        logoutButton.addActionListener(e -> {
            chatClientGUI.getConnectionManager().getWriter().println("logout " + chatClientGUI.getUsername());
            chatClientGUI.showLoginPage();
        });

        createRoomButton.addActionListener(e -> {
            String newRoom = JOptionPane.showInputDialog("Enter new room name:");
            if (newRoom != null && !newRoom.trim().isEmpty()) {
                newRoom = newRoom.replace("\n", "");
                chatClientGUI.getConnectionManager().getWriter().println("create " + newRoom);
                refreshChatRooms();
            }
        });

        refreshButton.addActionListener(e -> refreshChatRooms());

        JPanel leftTopPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        leftTopPanel.add(logoutButton);
        JPanel rightTopPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        rightTopPanel.add(loggedInLabel);
        rightTopPanel.add(refreshButton);

        topPanel.add(leftTopPanel, BorderLayout.WEST);
        topPanel.add(rightTopPanel, BorderLayout.EAST);

        add(scrollPane, BorderLayout.CENTER);
        add(topPanel, BorderLayout.NORTH);
        add(new JPanel(new FlowLayout(FlowLayout.CENTER)).add(createRoomButton), BorderLayout.SOUTH);

        refreshChatRooms();
    }

    private void refreshChatRooms() {
        chatRooms.clear();
        chatClientGUI.getConnectionManager().getWriter().println("list_rooms");
    }

    public static void updateChatRooms(String roomsResponse) {
        String[] parts = roomsResponse.split(" ");
        chatRooms.clear();
        for (int i = 1; i < parts.length; i++) {
            chatRooms.add(parts[i]);
        }
        // Update chat room buttons logic
    }
}
