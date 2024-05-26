import javax.swing.*;
import java.awt.*;

public class LoginPanel extends JPanel {
    public LoginPanel(ChatClientGUI chatClientGUI) {
        setLayout(new GridLayout(2, 1, 0, 10));
        JTextField nameField = new JTextField(15);
        JButton loginButton = new JButton("Login");
        loginButton.addActionListener(e -> {
            String username = nameField.getText().trim();
            if (!username.isEmpty()) {
                chatClientGUI.getConnectionManager().getWriter().println("login " + username);
                chatClientGUI.getConnectionManager().setUsername(username);
                chatClientGUI.showChatListPage();
            } else {
                JOptionPane.showMessageDialog(null, "Please enter a valid username.");
            }
        });

        JPanel inputPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 5, 5));
        inputPanel.add(new JLabel("Enter Username:"));
        inputPanel.add(nameField);
        add(inputPanel);
        add(loginButton);
    }
}
