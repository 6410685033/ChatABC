import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

public class ChatClientGUI extends JFrame {
    private String username;
    private Socket socket;
    private BufferedReader reader;
    private PrintWriter writer;
    private JTextArea chatArea;
    private JTextField messageField;
    private List<String> chatRooms;
    private String currentRoom;
    private List<String> currentParticipants;

    public ChatClientGUI() {
        setTitle("Chat Client");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(300, 150); // Adjusted size similar to an input dialog
        setLocationRelativeTo(null); // Center on the screen
        chatRooms = new ArrayList<>();
        currentParticipants = new ArrayList<>();
        showLoginPage();
    }

    // Step 1: Login Page
    private void showLoginPage() {
        JPanel loginPanel = new JPanel(new GridLayout(2, 1, 0, 10));
        JTextField nameField = new JTextField(15);
        JButton loginButton = new JButton("Login");

        loginButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                username = nameField.getText().trim();
                if (!username.isEmpty()) {
                    showChatListPage(); // Proceed to chat list page
                } else {
                    JOptionPane.showMessageDialog(null, "Please enter a valid username.");
                }
            }
        });

        JPanel inputPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 5, 5));
        inputPanel.add(new JLabel("Enter Username:"));
        inputPanel.add(nameField);
        loginPanel.add(inputPanel);
        loginPanel.add(loginButton);

        setContentPane(loginPanel);
        setVisible(true);
    }

    // Step 2: Chat List Page
    private void showChatListPage() {
        // Set a larger size for the main chat list
        setSize(500, 400);
        setLocationRelativeTo(null);

        JPanel listPanel = new JPanel(new BorderLayout());
        JPanel chatButtonPanel = new JPanel(new GridLayout(0, 1));
        JPanel topPanel = new JPanel(new BorderLayout());
        JButton createRoomButton = new JButton("Create New Room");
        JButton logoutButton = new JButton("Logout");
        JLabel loggedInLabel = new JLabel("Logged in as: " + username, JLabel.RIGHT);

        // Adjust logout button position to the top-left
        JPanel leftTopPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        leftTopPanel.add(logoutButton);

        // Adjust username display to the top-right
        JPanel rightTopPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        rightTopPanel.add(loggedInLabel);

        // Add both panels to the top panel
        topPanel.add(leftTopPanel, BorderLayout.WEST);
        topPanel.add(rightTopPanel, BorderLayout.EAST);

        // Create New Room Button
        createRoomButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String newRoom = JOptionPane.showInputDialog("Enter new room name:");
                if (newRoom != null && !newRoom.trim().isEmpty()) {
                    chatRooms.add(newRoom); // Add to the list locally
                    joinChatRoom(newRoom, true); // Join and create this room on the server
                }
            }
        });

        // Logout Button
        logoutButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                username = null;
                chatRooms.clear();
                showLoginPage(); // Return to the login page
            }
        });

        // Buttons for each available chat room
        for (String room : chatRooms) {
            JButton roomButton = new JButton(room);
            roomButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    joinChatRoom(room, false); // Join the existing chat room
                }
            });
            chatButtonPanel.add(roomButton);
        }

        // Arrange the bottom panel
        JPanel bottomPanel = new JPanel(new FlowLayout(FlowLayout.CENTER));
        bottomPanel.add(createRoomButton);

        listPanel.add(new JScrollPane(chatButtonPanel), BorderLayout.CENTER);
        listPanel.add(topPanel, BorderLayout.NORTH); // Add the top panel containing the logout button and username label
        listPanel.add(bottomPanel, BorderLayout.SOUTH);

        setContentPane(listPanel);
        setVisible(true);
    }

    // Step 3: Join a Chat Room
    private void joinChatRoom(String roomName, boolean create) {
        currentRoom = roomName;
        currentParticipants.clear();
        initializeConnection(create);
        showChatRoomPage();
    }

    // Initialize server connection and room joining
    private void initializeConnection(boolean create) {
        try {
            socket = new Socket("127.0.0.1", 7777); // Replace with the appropriate server IP/port
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            writer = new PrintWriter(socket.getOutputStream(), true);

            // Prefix "CREATE:" to request room creation
            String prefix = create ? "CREATE:" : "";
            writer.println(prefix + username + ":" + currentRoom); // Send username and room info

            new Thread(() -> {
                String response;
                try {
                    while ((response = reader.readLine()) != null) {
                        chatArea.append(response + "\n");

                        // Simulate receiving participant list updates
                        if (response.startsWith("Participants:")) {
                            String[] participants = response.substring(13).split(",");
                            currentParticipants.clear();
                            for (String participant : participants) {
                                currentParticipants.add(participant.trim());
                            }
                        }
                    }
                } catch (IOException e) {
                    chatArea.append("Disconnected from server.\n");
                }
            }).start();
        } catch (IOException e) {
            JOptionPane.showMessageDialog(this, "Error connecting to server: " + e.getMessage());
        }
    }

    // Step 4: Chat Room Page
    private void showChatRoomPage() {
        chatArea = new JTextArea();
        chatArea.setEditable(false);
        messageField = new JTextField();
        JButton sendButton = new JButton("Send");
        JButton leaveRoomButton = new JButton("Leave Room");
        JButton showParticipantsButton = new JButton("Show Participants");

        // Send Message Button
        sendButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String message = messageField.getText().trim();
                if (!message.isEmpty()) {
                    writer.println(username + " (" + currentRoom + "): " + message);
                    messageField.setText("");
                }
            }
        });

        // Leave Room Button
        leaveRoomButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try {
                    writer.close();
                    reader.close();
                    socket.close();
                } catch (IOException ex) {
                    ex.printStackTrace();
                }
                showChatListPage(); // Return to the chat list page
            }
        });

        // Show Participants Button
        showParticipantsButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                JOptionPane.showMessageDialog(null, "Participants: " + String.join(", ", currentParticipants));
            }
        });

        JPanel topPanel = new JPanel(new BorderLayout());
        topPanel.add(leaveRoomButton, BorderLayout.WEST);
        topPanel.add(showParticipantsButton, BorderLayout.EAST);

        JPanel inputPanel = new JPanel(new BorderLayout());
        inputPanel.add(messageField, BorderLayout.CENTER);
        inputPanel.add(sendButton, BorderLayout.EAST);

        JPanel chatPanel = new JPanel(new BorderLayout());
        chatPanel.add(new JScrollPane(chatArea), BorderLayout.CENTER);
        chatPanel.add(topPanel, BorderLayout.NORTH);
        chatPanel.add(inputPanel, BorderLayout.SOUTH);

        setContentPane(chatPanel);
        setVisible(true);
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new ChatClientGUI());
    }
}
