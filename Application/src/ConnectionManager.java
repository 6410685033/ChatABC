import javax.swing.*;
import java.io.*;
import java.net.Socket;

public class ConnectionManager {
    private Socket socket;
    private BufferedReader reader;
    private PrintWriter writer;
    private ChatClientGUI chatClientGUI;
    private String username;

    public ConnectionManager(ChatClientGUI chatClientGUI) {
        this.chatClientGUI = chatClientGUI;
        initializeConnection();
    }

    private void initializeConnection() {
        try {
            socket = new Socket("127.0.0.1", 7777);
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            writer = new PrintWriter(socket.getOutputStream(), true);
            new Thread(this::listenToServer).start();
        } catch (IOException e) {
            JOptionPane.showMessageDialog(chatClientGUI, "Error connecting to server: " + e.getMessage());
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
            e.printStackTrace();
        }
    }

    private void handleServerResponse(String response) {
        if (response.startsWith("list_rooms")) {
            SwingUtilities.invokeLater(() -> RoomListPanel.updateChatRooms(response));
        } else if (response.startsWith("attendances")) {
            SwingUtilities.invokeLater(() -> ChatRoomPanel.updateAttendances(response));
        } else if (response.startsWith("message")) {
            SwingUtilities.invokeLater(() -> ChatRoomPanel.updateMessage(response));
        }
    }

    public PrintWriter getWriter() {
        return writer;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }
}
