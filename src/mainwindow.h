
/**
 * @file mainwindow.h
 * @brief Header file for the MainWindow class, which serves as the main GUI for the application.
 * 
 * This file contains the declaration of the MainWindow class, which is responsible for managing
 * the application's graphical user interface (GUI). It includes methods for interacting with
 * the node canvas, managing finite state machine (FSM) clients, and handling Python process
 * communication.
 * 
 * The MainWindow class inherits from QMainWindow and provides slots for handling user interactions
 * and events related to the FSM editor and execution.
 * 
 * @author Josef Ambruz, Jakub Kovařík
 * @date 2025-5-11
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNodes/BasicGraphicsScene>
#include <QProcess>
#include <QTimer>
#include "DynamicPortsModel.hpp"

#include <QtNodes/ConnectionStyle>
#include <QtNodes/GraphicsView>
#include <QtNodes/StyleCollection>

#include <QAction>
#include <QFileDialog>
#include <QScreen>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QVBoxLayout>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QShortcut>


#include <QApplication>
#include "DynamicPortsModel.hpp"
#include "interpret_generator.h"
#include "qcombobox.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "spec_parser/automaton-data.hpp"
#include "client.hpp"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct VariableEntry {
    QHBoxLayout* layout;
    QLineEdit* lineEdit;
    QComboBox* dropDown;
    QString varValue;
};


/**
 * @class MainWindow
 * @brief The main application window for the FSM editor and runner.
 *
 * This class manages the GUI, node canvas, FSM client, and Python process communication.
 * It provides slots for user actions and updates the interface based on FSM and process events.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the MainWindow object.
     * @param parent The parent widget.
     */
    MainWindow(QWidget *parent = nullptr);
    /**
     * @brief Destructor for MainWindow.
     */
    ~MainWindow();

    void onAddWidget();

    void onRemoveWidget(const QString& varName);

public slots:

    /**
     * @brief Slot called when a node is clicked.
     * @param nodeId The ID of the clicked node.
     */
    void onNodeClicked(NodeId const nodeId);

    /**
     * @brief Slot called when node selection changes.
     */
    void onNodeSelectionChanged();
    /**
     * @brief Slot called when a connection is clicked.
     * @param connId The ID of the clicked connection.
     */
    void onConnectionClicked(ConnectionId connId);
    
    /**
     * @brief Slot called when the "Save to File" action is triggered.
     */
    void onSaveToFileClicked();

    /**
     * @brief Slot called when the "Load from File" action is triggered.
     */
    void onLoadFromFileClicked();

private slots:
    /**
     * @brief Slot for the "Add State" button click.
     */
    void on_button_addState_clicked();

    /**
     * @brief Slot for the "Run" button click.
     */
    void on_button_Run_clicked();

    /**
     * @brief Slot called when the action code text is changed.
     */
    void on_textEdit_actionCode_textChanged();

    /**
     * @brief Slot called when the state name line edit is changed.
     * @param arg1 The new state name.
     */
    void on_lineEdit_stateName_textChanged(const QString &arg1);

    /**
     * @brief Slot called when the connection condition text is changed.
     */
    void on_textEdit_connCond_textChanged();

    /**
     * @brief Slot called when the "Is Final" checkbox state changes.
     * @param arg1 The new state.
     */
    void on_checkBox_isFinal_stateChanged(int arg1);
    
    /**
     * @brief Slot for the "Set Start State" button click.
     */
    void on_pushButton_setStartState_clicked();

    /**
     * @brief Slot called when the FSM name line edit is changed.
     * @param arg1 The new FSM name.
     */
    void on_lineEdit_fsmName_textChanged(const QString &arg1);

    // Slots for FSM Client

    /**
     * @brief Slot called when the FSM client connects.
     */
    void onFsmClientConnected();

    /**
     * @brief Slot called when the FSM client disconnects.
     */
    void onFsmClientDisconnected();

    /**
     * @brief Slot called when a message is received from the FSM client.
     * @param msg The received JSON message.
     */
    void onFsmClientMessageReceived(const QJsonObject& msg);

    /**
     * @brief Slot called when an error occurs in the FSM client.
     * @param err The error message.
     */
    void onFsmClientError(const QString& err);

    // Slots for Python Process

    /**
     * @brief Slot called when the Python process finishes.
     * @param exitCode The exit code.
     * @param exitStatus The exit status.
     */
    void onPythonProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief Slot called when an error occurs in the Python process.
     * @param error The process error.
     */
    void onPythonProcessError(QProcess::ProcessError error);

    /**
     * @brief Slot called when the Python process state changes.
     * @param newState The new process state.
     */
    void onPythonProcessStateChanged(QProcess::ProcessState newState);


    /**
     * @brief Slot called when there is standard output from the Python process.
     */
    void onPythonReadyReadStdOut();

    /**
     * @brief Slot called when there is standard error output from the Python process.
     */
    void onPythonReadyReadStdErr();

    /**
     * @brief Slot for the "Stop" button click.
     */
    void on_button_Stop_clicked();

    /**
     * @brief Slot called when the transition delay spin box value changes.
     * @param arg1 The new value in milliseconds.
     */
    void on_spinBox_transDelayMs_valueChanged(int arg1);

private:
    void onVariableValueChangedByUser(const QString& varName, const QString& value);
    QMap<QString, VariableEntry> variables;
    std::vector<std::pair<std::string, std::string>> getVariableRowsAsVector();
    void onVariableUpdate(const QString& varName, const QString& value);

    Ui::MainWindow *ui;                      ///< The UI object.
    void initNodeCanvas();                   ///< Initializes the node canvas.
    void initializeModel();                  ///< Initializes the FSM model.
    void SetupUiByAutomaton(const Automaton& automaton);
    DynamicPortsModel* graphModel;           ///< The graph model for the node editor.
    QtNodes::BasicGraphicsScene* nodeScene;  ///< The graphics scene for the node editor.

    NodeId lastSelectedNode;                 ///< The last selected node ID.
    ConnectionId lastSelectedConnId;         ///< The last selected connection ID.

    FsmClient* fsmClient;                    ///< The FSM client for communication.
    QProcess* pythonFsmProcess;              ///< The Python FSM process.

    QString automatonName;                   ///< The name of the automaton.
    QString automatonDescription;            ///< The description of the automaton.

    QHash<QPushButton*, QHBoxLayout*> buttonToLayoutMap;
    QMap<QString, QLineEdit*> variableNameToLineEditMap;
};
#endif // MAINWINDOW_H
