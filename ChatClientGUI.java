import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.net.Socket;
import java.util.ArrayList;

public class ChatClientGUI extends JFrame {
    private Socket socket;
    private BufferedReader reader;
    private PrintWriter writer;
    private String username;
    private ArrayList<String> chatRooms = new ArrayList<>();
    private JTextArea chatArea;
    private JTextField messageField;
    private String currentRoom;

    public ChatClientGUI() {
        setTitle("Chat Client");
        setSize(500, 400);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        showUsernameInput();
    }

    private void showUsernameInput() {
        JPanel usernamePanel = new JPanel(new BorderLayout());
        JLabel label = new JLabel("Enter username:");
        JTextField usernameField = new JTextField(15);
        JButton continueButton = new JButton("Continue");

        continueButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                username = usernameField.getText();
                showRoomSelection();
            }
        });

        JPanel inputPanel = new JPanel();
        inputPanel.add(label);
        inputPanel.add(usernameField);
        inputPanel.add(continueButton);

        usernamePanel.add(inputPanel, BorderLayout.CENTER);
        setContentPane(usernamePanel);
        setVisible(true);
    }

    private void showRoomSelection() {
        JPanel roomPanel = new JPanel(new BorderLayout());
        JPanel roomButtons = new JPanel(new GridLayout(0, 1));
        JButton createRoomButton = new JButton("Create New Room");

        // Button to create a new chat room
        createRoomButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String newRoom = JOptionPane.showInputDialog("Enter new room name:");
                if (newRoom != null && !newRoom.trim().isEmpty()) {
                    chatRooms.add(newRoom);
                    joinRoom(newRoom);
                }
            }
        });

        // Buttons to join existing chat rooms
        for (String room : chatRooms) {
            JButton roomButton = new JButton(room);
            roomButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    joinRoom(room);
                }
            });
            roomButtons.add(roomButton);
        }

        roomPanel.add(new JScrollPane(roomButtons), BorderLayout.CENTER);
        roomPanel.add(createRoomButton, BorderLayout.SOUTH);
        setContentPane(roomPanel);
        setVisible(true);
    }

    private void joinRoom(String roomName) {
        currentRoom = roomName;
        initializeConnection();
        showChatRoom();
    }

    private void initializeConnection() {
        try {
            socket = new Socket("127.0.0.1", 7777);
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            writer = new PrintWriter(socket.getOutputStream(), true);
            writer.println(username + ":" + currentRoom); // Send username and room

            new Thread(() -> {
                String response;
                try {
                    while ((response = reader.readLine()) != null) {
                        chatArea.append(response + "\n");
                    }
                } catch (IOException e) {
                    chatArea.append("Disconnected from server.\n");
                }
            }).start();
        } catch (IOException e) {
            JOptionPane.showMessageDialog(this, "Error connecting to server: " + e.getMessage());
        }
    }

    private void showChatRoom() {
        JPanel chatPanel = new JPanel(new BorderLayout());
        chatArea = new JTextArea();
        chatArea.setEditable(false);
        messageField = new JTextField();
        JButton sendButton = new JButton("Send");
        JButton leaveRoomButton = new JButton("Leave Room");

        // Button to leave the chat room
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
                showRoomSelection();
            }
        });

        sendButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String message = messageField.getText();
                writer.println(username + " (" + currentRoom + "): " + message);
                messageField.setText("");
            }
        });

        JPanel buttonPanel = new JPanel(new BorderLayout());
        buttonPanel.add(sendButton, BorderLayout.EAST);
        buttonPanel.add(leaveRoomButton, BorderLayout.WEST);

        chatPanel.add(new JScrollPane(chatArea), BorderLayout.CENTER);
        chatPanel.add(messageField, BorderLayout.NORTH);
        chatPanel.add(buttonPanel, BorderLayout.SOUTH);
        setContentPane(chatPanel);
        setVisible(true);
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new ChatClientGUI());
    }
}
