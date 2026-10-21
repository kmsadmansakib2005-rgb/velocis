import java.awt.*;
import javax.swing.*;
import java.io.*;
import java.nio.*;

public class VelocisStudio {


    private static String runCompiler(String source) {
        try {
            ProcessBuilder builder =
                    new ProcessBuilder(
                            "C:\\Users\\USER\\Desktop\\Velocis\\velocis.exe"
                    );
            builder.redirectErrorStream(true);

            Process process = builder.start();

            process.getOutputStream().write(
                    source.getBytes(
                            java.nio.charset.StandardCharsets.UTF_8
                    )
            );
            process.getOutputStream().close();

            String output = new String(
                    process.getInputStream().readAllBytes(),
                    java.nio.charset.StandardCharsets.UTF_8
            );

            process.waitFor();

            return output;

        } catch (Exception e) {
            return "Compiler connection failed: "
                    + e.getMessage();
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(VelocisStudio::createGUI);
    }

    private static void createGUI() {

        JFrame frame = new JFrame("Velocis Mini | Compiler Studio");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(1100, 700);
        frame.setMinimumSize(new Dimension(850, 550));
        frame.setLocationRelativeTo(null);

        // Main container
        JPanel mainPanel = new JPanel(new BorderLayout(8, 8));
        mainPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

        // Header
        JLabel title = new JLabel("VELOCIS MINI  |  COMPILER STUDIO");
        title.setFont(new Font("Segoe UI", Font.BOLD, 22));

        JButton compileButton = new JButton("Compile");
        compileButton.setFocusable(false);
        JButton runButton = new JButton("Run");
        runButton.setFocusable(false);
        JButton lexicalButton = new JButton("Lexical Analysis");
        lexicalButton.setFocusable(false);

        JPanel actions = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        actions.add(lexicalButton);
        actions.add(compileButton);
        actions.add(runButton);

        JPanel header = new JPanel(new BorderLayout());
        header.add(title, BorderLayout.WEST);
        header.add(actions, BorderLayout.EAST);

        // Code editor
        JTextArea codeEditor = new JTextArea();
        codeEditor.setFont(new Font("Consolas", Font.PLAIN, 15));
        codeEditor.setTabSize(4);
        codeEditor.setText(
            "int x = 10;\n" +
            "x = x + 5;\n" +
            "print(x);\n"
        );

        JScrollPane editorScroll = new JScrollPane(codeEditor);
        editorScroll.setBorder(
            BorderFactory.createTitledBorder("Source Code (.vls)")
        );

        // AST panel
        JTextArea astArea = new JTextArea();
        astArea.setEditable(false);
        astArea.setFont(new Font("Consolas", Font.PLAIN, 14));
        astArea.setText("AST visualization will appear here.");

        JScrollPane astScroll = new JScrollPane(astArea);
        astScroll.setBorder(
            BorderFactory.createTitledBorder("AST Explorer")
        );

        // Editor and AST side by side
        JSplitPane workspace = new JSplitPane(
            JSplitPane.HORIZONTAL_SPLIT,
            editorScroll,
            astScroll
        );
        workspace.setResizeWeight(0.58);

        // Program output
        JTextArea outputArea = new JTextArea();
        outputArea.setEditable(false);
        outputArea.setFont(new Font("Consolas", Font.PLAIN, 14));
        outputArea.setText("Program output will appear here.");

        JScrollPane outputScroll = new JScrollPane(outputArea);
        outputScroll.setBorder(
            BorderFactory.createTitledBorder("Program Output")
        );
        outputScroll.setPreferredSize(new Dimension(0, 150));

        // Temporary button behavior

        compileButton.addActionListener(e -> {
            String result = runCompiler(codeEditor.getText());

            String startMarker = "----AST START----";
            String endMarker = "---AST END----";

            int start = result.indexOf(startMarker);
            int end = result.indexOf(endMarker);

            if (start >= 0 && end > start) {
                String ast = result.substring(
                        start + startMarker.length(), end
                ).trim();

                astArea.setText(ast);

                String programOutput =
                        result.substring(0, start).trim();

                outputArea.setText(programOutput);
            } else {
                astArea.setText("AST unavailable.");
                outputArea.setText(result);
            }
        });


        runButton.addActionListener(e ->
            outputArea.setText("Program execution will be connected next.")
        );

        lexicalButton.addActionListener(e ->{
            LexicalAnalysisWindow window= new LexicalAnalysisWindow();
            window.setLocationRelativeTo(frame);
            window.setVisible(true);
        });

        // Assemble the interface
        mainPanel.add(header, BorderLayout.NORTH);
        mainPanel.add(workspace, BorderLayout.CENTER);
        mainPanel.add(outputScroll, BorderLayout.SOUTH);

        frame.setContentPane(mainPanel);
        frame.setVisible(true);
    }
}
//TEST IMPORT EDITOR