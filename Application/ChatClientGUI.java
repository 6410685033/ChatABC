import javax.swing.*;

import Models.Chat;

import java.awt.*;
import java.awt.event.ActionEvent;
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
        setTitle("ChatABC");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(300, 150);
        setLocationRelativeTo(null);
        chatRooms = new ArrayList<>();
        currentParticipants = new ArrayList<>();
        initializeConnection();
        showLoginPage();
    }

    private void initializeConnection() {
        try {
            socket = new Socket("127.0.0.1", 7777);
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            writer = new PrintWriter(socket.getOutputStream(), true);
        } catch (IOException e) {
            JOptionPane.showMessageDialog(this, "Error connecting to server: " + e.getMessage());
            System.exit(1);
        }
    }

    private void showLoginPage() {
        JPanel loginPanel = new JPanel(new GridLayout(2, 1, 0, 10));
        JTextField nameField = new JTextField(15);
        JButton loginButton = new JButton("Login");
        loginButton.addActionListener(e -> {
            username = nameField.getText().trim();
            if (!username.isEmpty()) {
                writer.println("login " + username);
                showChatListPage();
            } else {
                JOptionPane.showMessageDialog(null, "Please enter a valid username.");
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

    private void showChatListPage() {
        setSize(500, 400);
    
        JPanel listPanel = new JPanel(new BorderLayout());
        JPanel chatButtonPanel = new JPanel(new GridLayout(0, 1));
        JPanel topPanel = new JPanel(new BorderLayout());
        JButton createRoomButton = new JButton("Create New Room");
        JButton logoutButton = new JButton("Logout");
        JButton refreshButton = new JButton("Refresh");
        JLabel loggedInLabel = new JLabel("Logged in as: " + username, JLabel.RIGHT);
    
        logoutButton.addActionListener(e -> {
            writer.println("logout " + username);
            username = null;
            setSize(300, 150);
            showLoginPage();
        });
    
        createRoomButton.addActionListener(e -> {
            String newRoom = JOptionPane.showInputDialog("Enter new room name:");
            if (newRoom != null && !newRoom.trim().isEmpty()) {
                writer.println("create " + newRoom);
                chatRooms.add(newRoom);
                refreshChatRooms();
            }
        });
    
        refreshButton.addActionListener(e -> {
            refreshChatRooms();
        });
    
        chatRooms.forEach(room -> {
            JButton roomButton = new JButton(room);
            roomButton.addActionListener(e -> joinChatRoom(room));
            chatButtonPanel.add(roomButton);
        });
    
        JPanel leftTopPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        leftTopPanel.add(logoutButton);
        JPanel rightTopPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        rightTopPanel.add(loggedInLabel);
        rightTopPanel.add(refreshButton);  // Add refresh button to the right top panel
    
        topPanel.add(leftTopPanel, BorderLayout.WEST);
        topPanel.add(rightTopPanel, BorderLayout.EAST);
    
        listPanel.add(new JScrollPane(chatButtonPanel), BorderLayout.CENTER);
        listPanel.add(topPanel, BorderLayout.NORTH);
        listPanel.add(new JPanel(new FlowLayout(FlowLayout.CENTER)).add(createRoomButton), BorderLayout.SOUTH);
    
        setContentPane(listPanel);
        setVisible(true);
    }
    
    private void refreshChatRooms() {
        chatRooms.clear();
        writer.println("list_rooms");
        // This assumes you have some mechanism on the server to handle "list_rooms" command and the client to handle the response
    }

    private void joinChatRoom(String roomName) {
        currentRoom = roomName;
        currentParticipants.clear();
        writer.println("join " + currentRoom + " " + username);
        showChatRoomPage();
    }

    private void showChatRoomPage() {
        chatArea = new JTextArea();
        chatArea.setEditable(false);
        messageField = new JTextField();
        JButton sendButton = new JButton("Send");
        JButton leaveRoomButton = new JButton("Leave Room");
        JButton showParticipantsButton = new JButton("Show Participants");

        sendButton.addActionListener(e -> {
            String message = messageField.getText().trim();
            if (!message.isEmpty()) {
                writer.println("message " + currentRoom + " " + username + " " + message);
                messageField.setText("");
            }
        });

        leaveRoomButton.addActionListener(e -> {
            writer.println("leave " + currentRoom + " " + username);
            showChatListPage();
        });

        showParticipantsButton.addActionListener(e -> {
            writer.println("attendances " + currentRoom);
            JOptionPane.showMessageDialog(null, "Participants: " + String.join(", ", currentParticipants));
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
        SwingUtilities.invokeLater(ChatClientGUI::new);
    }
}
