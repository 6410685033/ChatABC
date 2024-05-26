import javax.swing.*;
import java.awt.*;
import java.io.*;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

public class ChatClientGUI extends JFrame {
    private enum ViewState {
        LOGIN_VIEW, ROOM_LIST, ROOM
    }

    private ViewState currentState = ViewState.LOGIN_VIEW;

    private String username;
    private Socket socket;
    private BufferedReader reader;
    private PrintWriter writer;
    private JTextArea chatArea;
    private JTextField messageField;
    private List<String> chatRooms;
    private String currentRoom;
    private List<String> currentParticipants;

    private JPanel chatButtonPanel = new JPanel(new GridLayout(0, 1));

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
            new Thread(this::listenToServer).start();
        } catch (IOException e) {
            JOptionPane.showMessageDialog(this, "Error connecting to server: " + e.getMessage());
            System.exit(1);
        }
    }

    private void listenToServer() {
        String line;
        try {
            while ((line = reader.readLine()) != null) {
                handleServerResponse(line);
            }
        } catch (IOException e) {
            e.printStackTrace();  // Consider a reconnect or alerting the user 
        }
    }

    public void handleServerResponse(String response) {
        System.out.println("handleServerResponse occur: " + response);
        if (response.startsWith("list_rooms")) {
            if (currentState == ViewState.ROOM_LIST) {
                SwingUtilities.invokeLater(() -> updateChatRooms(response));
            }
        } else if (response.startsWith("attendances")) {
            if (currentState == ViewState.ROOM) {
                SwingUtilities.invokeLater(() -> updateAttendances(response));
            }
        }
    }

    private void updateAttendances(String attendancesResponse) {
        System.out.println("updateAttendances occur: " + attendancesResponse);
        String[] parts = attendancesResponse.split(" ");
        currentParticipants.clear();
        for (int i = 1; i < parts.length; i++) {
            currentParticipants.add(parts[i]);
        }
    }

    private void updateChatRooms(String roomsResponse) {
        System.out.println("updateChatRooms occur: " + roomsResponse);
        String[] parts = roomsResponse.split(" ");

        chatRooms.clear();
        for (int i = 1; i < parts.length; i++) {
            chatRooms.add(parts[i]);
        }
        
        if (currentState == ViewState.ROOM_LIST) {  // Only update the chat room list if the user is on the chat list screen
            updateChatRoomButtons();
        }
    }

    private void showLoginPage() {
        currentState = ViewState.LOGIN_VIEW;
        
        JPanel loginPanel = new JPanel(new GridLayout(2, 1, 0, 10));
        JTextField nameField = new JTextField(15);
        JButton loginButton = new JButton("Login");
        loginButton.addActionListener(e -> {
            username = nameField.getText().trim();
            if (!username.isEmpty()) {
                writer.println("login " + username);
                refreshChatRooms();
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
        currentState = ViewState.ROOM_LIST;
        setSize(500, 400);

        JPanel listPanel = new JPanel(new BorderLayout());
        JScrollPane scrollPane = new JScrollPane(chatButtonPanel); // Only one JScrollPane
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
                newRoom = newRoom.replace("\n", "");
                writer.println("create " + newRoom);
                chatRooms.add(newRoom);
                refreshChatRooms();
            }
        });

        refreshButton.addActionListener(e -> {
            refreshChatRooms();
        });

        updateChatRoomButtons();

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
    }

    private void updateChatRoomButtons() {
        chatButtonPanel.removeAll(); // Clear existing buttons
        for (String room : chatRooms) {
            JButton roomButton = new JButton(room);
            roomButton.addActionListener(e -> joinChatRoom(room));
            chatButtonPanel.add(roomButton);
        }
        chatButtonPanel.revalidate();
        chatButtonPanel.repaint();
    }

    private void joinChatRoom(String roomName) {
        currentState = ViewState.ROOM;
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

        JLabel roomLabel = new JLabel(currentRoom, JLabel.CENTER);
        topPanel.add(roomLabel, BorderLayout.CENTER);

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
