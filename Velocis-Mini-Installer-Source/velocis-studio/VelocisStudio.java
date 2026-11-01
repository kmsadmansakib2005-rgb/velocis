
import java.awt.*;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.List;
import javax.swing.*;
import javax.swing.border.*;

public class VelocisStudio {

    private static final Path COMPILER_PATH = locateCompiler();

    private static Path locateCompiler() {
        List<Path> candidates = new ArrayList<>();

        try {
            Path appLocation = Paths.get(
                    VelocisStudio.class.getProtectionDomain()
                            .getCodeSource().getLocation().toURI()
            ).toAbsolutePath();
            Path appDirectory = Files.isDirectory(appLocation)
                    ? appLocation : appLocation.getParent();

            if (appDirectory != null) {
                candidates.add(appDirectory.resolve("compiler").resolve("velocis.exe"));
                candidates.add(appDirectory.resolve("velocis.exe"));
                if (appDirectory.getParent() != null) {
                    candidates.add(appDirectory.getParent().resolve("velocis.exe"));
                }
            }
        } catch (Exception ignored) {
            // Fall back to the working directory for IDE development.
        }

        candidates.add(Paths.get("compiler", "velocis.exe").toAbsolutePath());
        candidates.add(Paths.get("velocis.exe").toAbsolutePath());

        for (Path candidate : candidates) {
            if (Files.isRegularFile(candidate)) {
                return candidate.normalize();
            }
        }

        return candidates.get(0).normalize();
    }

    // FOREST GRAPHITE THEME
    private static final Color BG = new Color(23, 26, 24);
    private static final Color PANEL = new Color(37, 45, 40);
    private static final Color PANEL_LIGHT = new Color(49, 59, 52);
    private static final Color EDITOR_BG = new Color(27, 32, 29);

    private static final Color TEXT = new Color(232, 238, 233);
    private static final Color MUTED = new Color(170, 183, 173);

    private static final Color ACCENT = new Color(117, 201, 154);
    private static final Color BLUE = new Color(125, 180, 205);
    private static final Color GREEN = new Color(117, 201, 154);
    private static final Color ERROR = new Color(235, 133, 125);

    private static final Color BORDER = new Color(65, 79, 69);

    private static JFrame frame;
    private static JTextArea editor;
    private static JTextArea outputArea;
    private static JTextArea activityArea;
    private static JTextField inputField;
    private static JLabel statusLabel;

    private static JButton buildButton;
    private static JButton sendButton;

    private static volatile Process currentProcess;

    private static String latestAST = null;

    public static void main(String[] args) {
        SwingUtilities.invokeLater(VelocisStudio::createGUI);
    }

    private static void createGUI() {



        frame = new JFrame("Velocis Mini | Compiler Studio");

        ImageIcon appIcon = new ImageIcon(
                VelocisStudio.class.getResource("Velocis-icon.png")
        );
        frame.setIconImage(appIcon.getImage());

        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(1350, 780);
        frame.setMinimumSize(new Dimension(1050, 650));
        frame.setLocationRelativeTo(null);

        JPanel root = new JPanel(new BorderLayout(12, 12));
        root.setBackground(BG);
        root.setBorder(new EmptyBorder(20, 22, 16, 22));

        // HEADER
        JPanel header = new JPanel(new BorderLayout());
        header.setOpaque(false);

        JPanel branding = new JPanel();
        branding.setOpaque(false);
        branding.setLayout(new BoxLayout(branding, BoxLayout.Y_AXIS));

        JLabel logo = new JLabel("VELOCIS MINI");
        logo.setFont(new Font("Segoe UI", Font.BOLD, 29));
        logo.setForeground(TEXT);

        JLabel subtitle = new JLabel(
                "COMPILER STUDIO | FROM CODE TO CLARITY"
        );
        subtitle.setFont(new Font("Segoe UI", Font.PLAIN, 14));
        subtitle.setForeground(MUTED);

        branding.add(logo);
        branding.add(Box.createVerticalStrut(5));
        branding.add(subtitle);

        JPanel actions = new JPanel(
                new FlowLayout(FlowLayout.RIGHT, 8, 5)
        );
        actions.setOpaque(false);

        JButton lexicalButton = createButton(
                "Lexical Analysis", PANEL_LIGHT, TEXT
        );

        JButton astButton = createButton(
                "AST Tree", PANEL_LIGHT, TEXT
        );

        buildButton = createButton(
                "Build & Run", ACCENT, BG
        );

        actions.add(lexicalButton);
        actions.add(astButton);
        actions.add(buildButton);

        header.add(branding, BorderLayout.WEST);
        header.add(actions, BorderLayout.EAST);

        // SOURCE EDITOR
        editor = new JTextArea();

        editor.setFont(new Font("Consolas", Font.PLAIN, 18));
        editor.setForeground(TEXT);
        editor.setBackground(EDITOR_BG);
        editor.setCaretColor(ACCENT);
        editor.setSelectionColor(new Color(58, 91, 70));
        editor.setSelectedTextColor(Color.WHITE);
        editor.setTabSize(4);
        editor.setBorder(new EmptyBorder(18, 18, 18, 18));

        editor.setText(
                "int x = 10;\n" +
                        "x = x + 5;\n" +
                        "print(x);\n"
        );

        JScrollPane editorScroll = new JScrollPane(editor);
        editorScroll.setBorder(
                createPanelBorder("SOURCE CODE  ·  .vls")
        );
        editorScroll.getViewport().setBackground(EDITOR_BG);

        // COMPILER ACTIVITY PANEL
        activityArea = new JTextArea();

        activityArea.setEditable(false);
        activityArea.setFont(new Font("Consolas", Font.PLAIN, 13));
        activityArea.setForeground(BLUE);
        activityArea.setBackground(EDITOR_BG);
        activityArea.setCaretColor(TEXT);
        activityArea.setLineWrap(true);
        activityArea.setWrapStyleWord(true);
        activityArea.setBorder(new EmptyBorder(14, 14, 14, 14));

        activityArea.setText(
                "Compiler activity will appear here.\n\n" +
                        "Run your program to see parser activity,\n" +
                        "syntax errors and panic-mode recovery."
        );

        JScrollPane activityScroll = new JScrollPane(activityArea);
        activityScroll.setBorder(
                createPanelBorder("COMPILER ACTIVITY")
        );
        activityScroll.getViewport().setBackground(EDITOR_BG);

        // EDITOR + ACTIVITY LAYOUT
        JSplitPane editorActivitySplit = new JSplitPane(
                JSplitPane.HORIZONTAL_SPLIT,
                editorScroll,
                activityScroll
        );

        editorActivitySplit.setResizeWeight(0.70);
        editorActivitySplit.setDividerLocation(0.70);
        editorActivitySplit.setBorder(null);
        editorActivitySplit.setBackground(BG);
        editorActivitySplit.setContinuousLayout(true);

        // COMPILER OUTPUT
        outputArea = new JTextArea();

        outputArea.setEditable(false);
        outputArea.setFont(new Font("Consolas", Font.PLAIN, 16));
        outputArea.setForeground(ACCENT);
        outputArea.setBackground(EDITOR_BG);
        outputArea.setCaretColor(TEXT);
        outputArea.setSelectionColor(new Color(58, 91, 70));
        outputArea.setBorder(new EmptyBorder(14, 14, 14, 14));

        outputArea.setText(
                "Ready. Write your Velocis program and click Build & Run."
        );

        JScrollPane outputScroll = new JScrollPane(outputArea);
        outputScroll.setBorder(
                createPanelBorder("COMPILER OUTPUT")
        );
        outputScroll.getViewport().setBackground(EDITOR_BG);

        // RUNTIME INPUT
        inputField = new JTextField();

        inputField.setFont(new Font("Consolas", Font.PLAIN, 16));
        inputField.setForeground(TEXT);
        inputField.setBackground(PANEL_LIGHT);
        inputField.setCaretColor(ACCENT);
        inputField.setBorder(new CompoundBorder(
                new LineBorder(BORDER, 1, true),
                new EmptyBorder(9, 12, 9, 12)
        ));

        sendButton = createButton(
                "Send Input", GREEN, Color.BLACK
        );

        JPanel inputPanel = new JPanel(new BorderLayout(10, 0));
        inputPanel.setBackground(PANEL);
        inputPanel.setBorder(new EmptyBorder(12, 12, 12, 12));

        JLabel inputLabel = new JLabel("RUNTIME INPUT");
        inputLabel.setFont(new Font("Segoe UI", Font.BOLD, 13));
        inputLabel.setForeground(MUTED);

        inputPanel.add(inputLabel, BorderLayout.WEST);
        inputPanel.add(inputField, BorderLayout.CENTER);
        inputPanel.add(sendButton, BorderLayout.EAST);

        JPanel console = new JPanel(new BorderLayout(0, 8));
        console.setOpaque(false);
        console.add(outputScroll, BorderLayout.CENTER);
        console.add(inputPanel, BorderLayout.SOUTH);
        console.setPreferredSize(new Dimension(0, 240));

        // MAIN WORKSPACE
        JSplitPane workspace = new JSplitPane(
                JSplitPane.VERTICAL_SPLIT,
                editorActivitySplit,
                console
        );

        workspace.setResizeWeight(0.65);
        workspace.setDividerLocation(0.65);
        workspace.setBorder(null);
        workspace.setBackground(BG);
        workspace.setContinuousLayout(true);

        // STATUS BAR
        JPanel statusBar = new JPanel(new BorderLayout());
        statusBar.setBackground(PANEL);
        statusBar.setBorder(new EmptyBorder(10, 12, 10, 12));

        statusLabel = new JLabel("●  READY");
        statusLabel.setFont(new Font("Segoe UI", Font.BOLD, 13));
        statusLabel.setForeground(GREEN);

        JLabel version = new JLabel(
                "VELOCIS MINI  |  COMPILER STUDIO"
        );
        version.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        version.setForeground(MUTED);

        statusBar.add(statusLabel, BorderLayout.WEST);
        statusBar.add(version, BorderLayout.EAST);

        // COMPLETE LAYOUT
        root.add(header, BorderLayout.NORTH);
        root.add(workspace, BorderLayout.CENTER);

        JPanel complete = new JPanel(new BorderLayout(0, 10));
        complete.setBackground(BG);
        complete.add(root, BorderLayout.CENTER);
        complete.add(statusBar, BorderLayout.SOUTH);

        frame.setContentPane(complete);

        // ACTIONS
        buildButton.addActionListener(e -> buildAndRun());

        lexicalButton.addActionListener(e -> showLexicalAnalysis());

        astButton.addActionListener(e -> showAST());

        sendButton.addActionListener(e -> sendRuntimeInput());

        inputField.addActionListener(e -> sendRuntimeInput());

        frame.setVisible(true);
    }

    // BUILD AND RUN
    private static void buildAndRun() {

        if (currentProcess != null && currentProcess.isAlive()) {
            JOptionPane.showMessageDialog(
                    frame,
                    "A Velocis program is already running."
            );
            return;
        }

        if (!Files.isRegularFile(COMPILER_PATH)) {
            JOptionPane.showMessageDialog(
                    frame,
                    "Compiler not found:\n" + COMPILER_PATH,
                    "Compiler Error",
                    JOptionPane.ERROR_MESSAGE
            );
            return;
        }

        String source = editor.getText();

        if (source.isBlank()) {
            JOptionPane.showMessageDialog(frame, "Source code is empty.");
            return;
        }

        buildButton.setEnabled(false);
        sendButton.setEnabled(false);
        latestAST = null;

        outputArea.setText("");
        activityArea.setText("Starting compiler...\n");

        statusLabel.setText("●  COMPILING...");
        statusLabel.setForeground(BLUE);

        Thread worker = new Thread(() -> {

            Path tempFile = null;

            StringBuilder standardOutput = new StringBuilder();
            StringBuilder standardError = new StringBuilder();

            try {
                tempFile = writeTemporarySource(source);

                ProcessBuilder builder = new ProcessBuilder(
                        COMPILER_PATH.toString(),
                        "--ast",
                        tempFile.toAbsolutePath().toString()
                );

                // Keep stdout and stderr separate.
                builder.redirectErrorStream(false);

                Process process = builder.start();
                currentProcess = process;

                SwingUtilities.invokeLater(
                        () -> sendButton.setEnabled(true)
                );

                // Read stdout in a separate thread.
                Thread outputReader = new Thread(() -> {

                    try (BufferedReader reader = new BufferedReader(
                            new InputStreamReader(
                                    process.getInputStream(),
                                    StandardCharsets.UTF_8
                            ))) {

                        String line;

                        while ((line = reader.readLine()) != null) {

                            synchronized (standardOutput) {
                                standardOutput.append(line).append("\n");
                            }

                            String snapshot;

                            synchronized (standardOutput) {
                                snapshot = standardOutput.toString();
                            }

                            SwingUtilities.invokeLater(
                                    () -> updateBuildOutput(snapshot)
                            );
                        }

                    } catch (IOException ex) {
                        appendActivity(
                                "Output stream error: " + ex.getMessage()
                        );
                    }

                });

                // Read stderr in another thread.
                Thread errorReader = new Thread(() -> {

                    try (BufferedReader reader = new BufferedReader(
                            new InputStreamReader(
                                    process.getErrorStream(),
                                    StandardCharsets.UTF_8
                            ))) {

                        String line;

                        while ((line = reader.readLine()) != null) {

                            synchronized (standardError) {
                                standardError.append(line).append("\n");
                            }

                            String snapshot;

                            synchronized (standardError) {
                                snapshot = standardError.toString();
                            }

                            SwingUtilities.invokeLater(
                                    () -> updateActivity(snapshot)
                            );
                        }

                    } catch (IOException ex) {
                        appendActivity(
                                "Activity stream error: " + ex.getMessage()
                        );
                    }

                });

                outputReader.start();
                errorReader.start();

                int exitCode = process.waitFor();

                outputReader.join();
                errorReader.join();

                String finalOutput;
                String finalActivity;

                synchronized (standardOutput) {
                    finalOutput = standardOutput.toString();
                }

                synchronized (standardError) {
                    finalActivity = standardError.toString();
                }

                SwingUtilities.invokeLater(() -> {

                    updateBuildOutput(finalOutput);
                    updateActivity(finalActivity);

                    statusLabel.setText(
                            exitCode == 0
                                    ? "●  PROCESS FINISHED"
                                    : "●  PROCESS FINISHED WITH ERRORS"
                    );

                    statusLabel.setForeground(
                            exitCode == 0 ? GREEN : ERROR
                    );

                    buildButton.setEnabled(true);
                    sendButton.setEnabled(false);
                    currentProcess = null;
                });

            } catch (Exception ex) {

                SwingUtilities.invokeLater(() -> {

                    outputArea.setText(
                            "Compiler connection failed:\n" + ex.getMessage()
                    );

                    activityArea.setText(
                            "Compiler process could not be started."
                    );

                    statusLabel.setText("●  CONNECTION FAILED");
                    statusLabel.setForeground(ERROR);

                    buildButton.setEnabled(true);
                    sendButton.setEnabled(false);
                    currentProcess = null;
                });

            } finally {

                if (tempFile != null) {
                    try {
                        Files.deleteIfExists(tempFile);
                    } catch (IOException ignored) {
                    }
                }
            }

        });

        worker.setDaemon(true);
        worker.start();
    }

    // DISPLAY STANDARD OUTPUT
    private static void updateBuildOutput(String result) {

        String startMarker = "----AST START----";
        String endMarker = "---AST END----";

        int start = result.indexOf(startMarker);
        int end = result.indexOf(endMarker);

        StringBuilder display = new StringBuilder();

        if (start >= 0 && end > start) {

            latestAST = result.substring(
                    start + startMarker.length(),
                    end
            ).trim();

            display.append(
                    result.substring(end + endMarker.length())
            );

        } else if (start < 0) {

            display.append(result);

        }

        // Remove the compiler banner from the output panel.
        String cleanedOutput = display.toString()
                .replace("Velocis Compiler.\n", "")
                .trim();

        outputArea.setText(cleanedOutput);

        outputArea.setCaretPosition(
                outputArea.getDocument().getLength()
        );
    }

    // DISPLAY STANDARD ERROR / COMPILER ACTIVITY
    private static void updateActivity(String result) {

        activityArea.setText(result);

        activityArea.setCaretPosition(
                activityArea.getDocument().getLength()
        );
    }

    private static void appendActivity(String message) {

        SwingUtilities.invokeLater(() -> {
            activityArea.append(message + "\n");

            activityArea.setCaretPosition(
                    activityArea.getDocument().getLength()
            );
        });
    }

    // LEXICAL ANALYSIS
    private static void showLexicalAnalysis() {

        if (!Files.isRegularFile(COMPILER_PATH)) {
            JOptionPane.showMessageDialog(
                    frame,
                    "Compiler executable was not found."
            );
            return;
        }

        String source = editor.getText();

        if (source.isBlank()) {
            JOptionPane.showMessageDialog(frame, "Source code is empty.");
            return;
        }

        statusLabel.setText("●  RUNNING LEXER...");
        statusLabel.setForeground(BLUE);

        Thread worker = new Thread(() -> {

            Path tempFile = null;
            List<String[]> tokens = new ArrayList<>();
            StringBuilder diagnostics = new StringBuilder();

            try {
                tempFile = writeTemporarySource(source);

                ProcessBuilder builder = new ProcessBuilder(
                        COMPILER_PATH.toString(),
                        "--tokens",
                        tempFile.toAbsolutePath().toString()
                );

                builder.redirectErrorStream(true);

                Process process = builder.start();

                try (BufferedReader reader = new BufferedReader(
                        new InputStreamReader(
                                process.getInputStream(),
                                StandardCharsets.UTF_8
                        ))) {

                    String line;

                    while ((line = reader.readLine()) != null) {

                        if (line.startsWith("TOKEN\t")) {

                            String[] parts = line.split("\t", 4);

                            if (parts.length == 4) {
                                tokens.add(new String[]{
                                        parts[1],
                                        parts[2],
                                        parts[3]
                                });
                            }

                        } else if (!line.equals("Velocis Compiler.")) {

                            diagnostics.append(line).append("\n");
                        }
                    }
                }

                int exitCode = process.waitFor();

                List<String[]> finalTokens = tokens;
                String finalDiagnostics = diagnostics.toString();

                SwingUtilities.invokeLater(() -> {

                    LexicalAnalysisWindow window =
                            new LexicalAnalysisWindow(
                                    finalTokens,
                                    finalDiagnostics
                            );

                    window.setLocationRelativeTo(frame);
                    window.setVisible(true);

                    statusLabel.setText(
                            exitCode == 0
                                    ? "●  LEXICAL ANALYSIS COMPLETE"
                                    : "●  LEXICAL ERRORS FOUND"
                    );

                    statusLabel.setForeground(
                            exitCode == 0 ? GREEN : ERROR
                    );
                });

            } catch (Exception ex) {

                SwingUtilities.invokeLater(() -> {

                    JOptionPane.showMessageDialog(
                            frame,
                            "Lexical analysis failed:\n" + ex.getMessage(),
                            "Lexer Error",
                            JOptionPane.ERROR_MESSAGE
                    );

                    statusLabel.setText("●  LEXER FAILED");
                    statusLabel.setForeground(ERROR);
                });

            } finally {

                if (tempFile != null) {
                    try {
                        Files.deleteIfExists(tempFile);
                    } catch (IOException ignored) {
                    }
                }
            }

        });

        worker.setDaemon(true);
        worker.start();
    }

    // AST WINDOW
    private static void showAST() {

        if (latestAST == null || latestAST.isBlank()) {
            JOptionPane.showMessageDialog(
                    frame,
                    "No AST is available yet.\nBuild a program successfully first.",
                    "AST Unavailable",
                    JOptionPane.INFORMATION_MESSAGE
            );
            return;
        }

        JFrame astFrame = new JFrame("Velocis Mini | AST Tree");
        astFrame.setIconImage(frame.getIconImage());
        astFrame.setSize(850, 650);
        astFrame.setMinimumSize(new Dimension(600, 450));
        astFrame.setLocationRelativeTo(frame);

        JTextArea astArea = new JTextArea(latestAST);

        astArea.setEditable(false);
        astArea.setFont(new Font("Consolas", Font.PLAIN, 17));
        astArea.setForeground(TEXT);
        astArea.setBackground(EDITOR_BG);
        astArea.setCaretColor(ACCENT);
        astArea.setBorder(new EmptyBorder(18, 18, 18, 18));

        JScrollPane scroll = new JScrollPane(astArea);
        scroll.setBorder(
                createPanelBorder("ABSTRACT SYNTAX TREE")
        );
        scroll.getViewport().setBackground(EDITOR_BG);

        astFrame.getContentPane().setBackground(BG);
        astFrame.add(scroll);
        astFrame.setVisible(true);
    }

    // RUNTIME INPUT
    private static void sendRuntimeInput() {

        String input = inputField.getText();

        if (input.isEmpty()) {
            return;
        }

        Process process = currentProcess;

        if (process == null || !process.isAlive()) {
            outputArea.append(
                    "\nNo running program is waiting for input.\n"
            );
            return;
        }

        try {
            BufferedWriter writer = new BufferedWriter(
                    new OutputStreamWriter(
                            process.getOutputStream(),
                            StandardCharsets.UTF_8
                    )
            );

            writer.write(input);
            writer.newLine();
            writer.flush();

            outputArea.append("\n> " + input + "\n");
            inputField.setText("");

        } catch (IOException ex) {

            outputArea.append(
                    "\nCould not send input: " + ex.getMessage() + "\n"
            );
        }
    }

    // TEMPORARY SOURCE FILE
    private static Path writeTemporarySource(String source)
            throws IOException {

        Path file = Files.createTempFile("velocis-", ".vls");

        Files.writeString(
                file,
                source,
                StandardCharsets.UTF_8
        );

        return file;
    }

    // UI HELPERS
    private static JButton createButton(
            String text,
            Color background,
            Color foreground
    ) {

        JButton button = new JButton(text);

        button.setFont(new Font("Segoe UI", Font.BOLD, 14));
        button.setForeground(foreground);
        button.setBackground(background);
        button.setFocusPainted(false);
        button.setBorder(new EmptyBorder(11, 16, 11, 16));
        button.setCursor(new Cursor(Cursor.HAND_CURSOR));
        button.setOpaque(true);

        return button;
    }

    private static TitledBorder createPanelBorder(String title) {

        return BorderFactory.createTitledBorder(
                new LineBorder(BORDER, 1, true),
                title,
                TitledBorder.LEFT,
                TitledBorder.TOP,
                new Font("Segoe UI", Font.BOLD, 14),
                ACCENT
        );
    }
}
