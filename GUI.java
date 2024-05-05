// GUI.java
import javax.swing.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;

public class GUI extends JFrame {
    private JTextField hostnameField;
    private JTextField nameField;
    private JTextField surnameField;
    private JTextField birthdateField;
    private JTextArea responseArea;
    private JButton sendButton;

    private static final int PORTNO1 = 15139;

    public GUI() {
        // Initialize GUI components
        setTitle("Client Interaction");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(400, 400);

        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
        hostnameField = new JTextField("localhost", 20);
        nameField = new JTextField("Enter Name", 20);
        surnameField = new JTextField("Enter Surname", 20);
        birthdateField = new JTextField("Enter Birthdate (BE)", 20);
        responseArea = new JTextArea(10, 30);
        responseArea.setEditable(false);
        sendButton = new JButton("Send");

        panel.add(new JLabel("Server Hostname:"));
        panel.add(hostnameField);
        panel.add(new JLabel("Name:"));
        panel.add(nameField);
        panel.add(new JLabel("Surname:"));
        panel.add(surnameField);
        panel.add(new JLabel("Birthdate (BE):"));
        panel.add(birthdateField);
        panel.add(sendButton);
        panel.add(new JScrollPane(responseArea));

        add(panel);
        pack();
        setLocationRelativeTo(null);

        // Set the send button action
        sendButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                handleSendButton();
            }
        });
    }

    private void handleSendButton() {
        String hostname = hostnameField.getText();
        String name = nameField.getText();
        String surname = surnameField.getText();
        String birthdate = birthdateField.getText();

        // Connect to server and send data
        try (Socket socket = new Socket(hostname, PORTNO1);
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))) {

            // Send the data
            String data = String.format("%s %s\n%s\n", name, surname, birthdate);
            out.println(data);

            // Read server response
            String response = in.readLine();
            responseArea.setText("Server Response: " + response);
        } catch (IOException ex) {
            responseArea.setText("Error: " + ex.getMessage());
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new GUI().setVisible(true));
    }
}
