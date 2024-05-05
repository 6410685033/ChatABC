// CollaborativeTextEditorGUI.java
import javax.swing.*;
import java.awt.*;
import java.io.*;
import java.util.Timer;
import java.util.TimerTask;

public class CollaborativeTextEditorGUI extends JFrame {
    private JTextArea textArea;

    public CollaborativeTextEditorGUI() {
        // Initialize GUI components
        textArea = new JTextArea(20, 50);
        JScrollPane scrollPane = new JScrollPane(textArea);
        add(scrollPane);

        // Schedule a periodic task to update text area
        Timer timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                updateTextArea();
            }
        }, 0, 1000); // Every second

        // Finalize JFrame setup
        setTitle("Collaborative Text Editor (GUI)");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        pack();
        setLocationRelativeTo(null);
        setVisible(true);
    }

    private void updateTextArea() {
        try (BufferedReader reader = new BufferedReader(new FileReader("shared_document.txt"))) {
            StringBuilder content = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line).append("\n");
            }
            SwingUtilities.invokeLater(() -> textArea.setText(content.toString()));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(CollaborativeTextEditorGUI::new);
    }
}
