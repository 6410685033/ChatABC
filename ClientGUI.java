import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.net.Socket;

public class ClientGUI extends JFrame {
    private Socket socket;
    private BufferedReader reader;
    private PrintWriter writer;
    private JTextArea chatArea;
    private JTextField messageField;
    private JTextField nameField;

    public ClientGUI() {
        setTitle("Client Chat");
        setSize(500, 400);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout());

        chatArea = new JTextArea();
        chatArea.setEditable(false);
        JScrollPane chatScrollPane = new JScrollPane(chatArea);
        add(chatScrollPane, BorderLayout.CENTER);

        JPanel inputPanel = new JPanel(new BorderLayout());
        messageField = new JTextField();
        JButton sendButton = new JButton("Send");

        sendButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                sendMessage();
            }
        });

        inputPanel.add(messageField, BorderLayout.CENTER);
        inputPanel.add(sendButton, BorderLayout.EAST);
        add(inputPanel, BorderLayout.SOUTH);

        JPanel namePanel = new JPanel(new BorderLayout());
        nameField = new JTextField();
        nameField.setText("Enter your name here...");
        JButton connectButton = new JButton("Connect");

        connectButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                connectToServer();
            }
        });

        namePanel.add(nameField, BorderLayout.CENTER);
        namePanel.add(connectButton, BorderLayout.EAST);
        add(namePanel, BorderLayout.NORTH);

        setVisible(true);
    }

    private void connectToServer() {
        try {
            socket = new Socket("127.0.0.1", 7777); // Adjust IP address and port if needed
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            writer = new PrintWriter(socket.getOutputStream(), true);

            String name = nameField.getText();
            writer.println(name); // Send the client name initially
            chatArea.append("Connected as: " + name + "\n");

            new Thread(() -> {
                String response;
                try {
                    while ((response = reader.readLine()) != null) {
                        chatArea.append("Server: " + response + "\n");
                    }
                } catch (IOException e) {
                    chatArea.append("Disconnected from server.\n");
                    System.err.println("Error while reading from server: " + e.getMessage());
                }
            }).start();
        } catch (IOException e) {
            chatArea.append("Could not connect to server.\n");
            System.err.println("Error while connecting: " + e.getMessage());
        }
    }

    private void sendMessage() {
        String message = messageField.getText();
        if (writer != null) {
            writer.println(message);
            chatArea.append("Me: " + message + "\n");
            messageField.setText("");
        } else {
            chatArea.append("Unable to send message: not connected to server.\n");
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new ClientGUI());
    }
}
