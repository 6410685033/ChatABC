import javax.swing.*;

public class ChatClientGUI extends JFrame {
    private ConnectionManager connectionManager;

    public ChatClientGUI() {
        setTitle("ChatABC");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(300, 150);
        setLocationRelativeTo(null);
        connectionManager = new ConnectionManager(this);
        showLoginPage();
    }

    public ConnectionManager getConnectionManager() {
        return connectionManager;
    }

    public String getUsername() {
        return connectionManager.getUsername();
    }

    public void showLoginPage() {
        setContentPane(new LoginPanel(this));
        setVisible(true);
    }

    public void showChatListPage() {
        setContentPane(new RoomListPanel(this));
        setSize(500, 400);
        setVisible(true);
    }

    public void showChatRoomPage(String roomName) {
        setContentPane(new ChatRoomPanel(this, roomName));
        setVisible(true);
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(ChatClientGUI::new);
    }
}
