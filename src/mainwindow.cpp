#include "mainwindow.h"
#include "./ui_mainwindow.h"

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


#include <QApplication>
#include "DynamicPortsModel.hpp"
#include "interpret_generator.h"
#include "spec_parser/automaton-data.hpp"
#include "client.hpp"

using QtNodes::BasicGraphicsScene;
using QtNodes::ConnectionStyle;
using QtNodes::GraphicsView;
using QtNodes::NodeRole;
using QtNodes::StyleCollection;
using QtNodes::NodeId;

void MainWindow::initializeModel()
{
    NodeId id1 = graphModel->addNode();
    graphModel->setNodeData(id1, NodeRole::Position, QPointF(0, 0));
    graphModel->setNodeData(id1, NodeRole::InPortCount, 1);
    graphModel->setNodeData(id1, NodeRole::OutPortCount, 1);

    NodeId id2 = graphModel->addNode();
    graphModel->setNodeData(id2, NodeRole::Position, QPointF(300, 300));
    graphModel->setNodeData(id2, NodeRole::InPortCount, 1);
    graphModel->setNodeData(id2, NodeRole::OutPortCount, 1);

    graphModel->addConnection(ConnectionId{id1, 0, id2, 0});
}

QMenuBar *createSaveRestoreMenu(DynamicPortsModel &graphModel,
                                BasicGraphicsScene *scene,
                                GraphicsView &view)
{
    auto menuBar = new QMenuBar();
    QMenu *menu = menuBar->addMenu("File");
    auto saveAction = menu->addAction("Save Scene");
    auto loadAction = menu->addAction("Load Scene");

    QObject::connect(saveAction, &QAction::triggered, scene, [&graphModel] {
        QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                        "Open Flow Scene",
                                                        QDir::homePath(),
                                                        "Flow Scene Files (*.flow)");

        if (!fileName.isEmpty()) {
            if (!fileName.endsWith("flow", Qt::CaseInsensitive))
                fileName += ".flow";

            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(QJsonDocument(graphModel.save()).toJson());
            }
        }
    });

    QObject::connect(loadAction, &QAction::triggered, scene, [&graphModel, &view, scene] {
        QString fileName = QFileDialog::getOpenFileName(nullptr,
                                                        "Open Flow Scene",
                                                        QDir::homePath(),
                                                        "Flow Scene Files (*.flow)");
        if (!QFileInfo::exists(fileName))
            return;

        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly))
            return;

        scene->clearScene();

        QByteArray const wholeFile = file.readAll();

        graphModel.load(QJsonDocument::fromJson(wholeFile).object());

        view.centerScene();
    });

    return menuBar;
}


QAction *createNodeAction(DynamicPortsModel &graphModel, GraphicsView &view)
{
    auto action = new QAction(QStringLiteral("Create Node"), &view);
    QObject::connect(action, &QAction::triggered, [&]() {
        // Mouse position in scene coordinates.
        QPointF posView = view.mapToScene(view.mapFromGlobal(QCursor::pos()));

        NodeId const newId = graphModel.addNode();
        graphModel.setNodeData(newId, NodeRole::Position, posView);
    });

    return action;
}

void MainWindow::initNodeCanvas()
{
    graphModel = new DynamicPortsModel();
    // Initialize and connect two nodes.
    initializeModel();

    // Create the QtNode scene
    nodeScene = new BasicGraphicsScene(*graphModel, this);

    qWarning() << "MODEF FROM SCENE " << &(nodeScene->graphModel());

    // Create a View for the scene
    auto* view = new GraphicsView(nodeScene, this);
    view->setContextMenuPolicy(Qt::ActionsContextMenu);
    view->insertAction(view->actions().front(), createNodeAction(*graphModel, *view));

    // Add the view with the QtNode scene to our UI
    auto* layout = new QVBoxLayout(ui->nodeCanvasContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    // client and fsm interpret initialization
    , fsmClient(new FsmClient(this))
    , pythonFsmProcess(new QProcess(this))
{
    ui->setupUi(this);
    initNodeCanvas();

    connect(nodeScene, &BasicGraphicsScene::nodeClicked, this, &MainWindow::onNodeClicked);
    connect(nodeScene, &BasicGraphicsScene::selectionChanged, this, &MainWindow::onNodeSelectionChanged);
    connect(nodeScene, &BasicGraphicsScene::connectionClicked, this, & MainWindow::onConnectionClicked);

    // --- Connect FsmClient signals ---
    connect(fsmClient, &FsmClient::connected, this, &MainWindow::onFsmClientConnected);
    connect(fsmClient, &FsmClient::disconnected, this, &MainWindow::onFsmClientDisconnected);
    connect(fsmClient, &FsmClient::messageReceived, this, &MainWindow::onFsmClientMessageReceived);
    connect(fsmClient, &FsmClient::fsmError, this, &MainWindow::onFsmClientError);

    // --- Connect QProcess signals ---
    connect(pythonFsmProcess, &QProcess::finished, this, &MainWindow::onPythonProcessFinished);
    connect(pythonFsmProcess, &QProcess::errorOccurred, this, &MainWindow::onPythonProcessError);
    connect(pythonFsmProcess, &QProcess::stateChanged, this, &MainWindow::onPythonProcessStateChanged);
    connect(pythonFsmProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::onPythonReadyReadStdOut);
    connect(pythonFsmProcess, &QProcess::readyReadStandardError, this, &MainWindow::onPythonReadyReadStdErr);
}

MainWindow::~MainWindow()
{
    // Ensure Python process is terminated if running
    if (pythonFsmProcess && pythonFsmProcess->state() != QProcess::NotRunning) {
        qInfo() << "MainWindow destructor: Terminating Python FSM process.";
        pythonFsmProcess->terminate();
        if (!pythonFsmProcess->waitForFinished(1000)) {
            pythonFsmProcess->kill();
            pythonFsmProcess->waitForFinished();
        }
    }
    // FsmClient and QProcess will be deleted due to parent-child relationship
    // if fsmClient is connected, its destructor should handle disconnection, hopefully.
    delete ui;
}


/**
 *    FSMCLIENT SLOTS
 *  ========================================================================
 *  ========================================================================
 */

// TODO: this part was written with help of LLM and should be taken with grain
// of salt, it only models the supposed functionality
void MainWindow::onFsmClientConnected() {
    qInfo() << "[MainWindow] Connected to FSM server!";
    // Update UI (e.g., enable "Send Variable" button, change status label)
    ui->textEdit_logOut->append("CLIENT: Connected to FSM server.");
}

void MainWindow::onFsmClientDisconnected() {
    qInfo() << "[MainWindow] Disconnected from FSM server.";
    // Update UI
    ui->textEdit_logOut->append("CLIENT: Disconnected from FSM server.");
}

void MainWindow::onFsmClientMessageReceived(const QJsonObject& msg) {
    qInfo() << "[MainWindow] Message from FSM:" << msg;
    // Update your UI based on the message
    // Example: if msg["type"] == "CURRENT_STATE", update a label with msg["payload"]["name"]
    QJsonDocument doc(msg);
    ui->textEdit_logOut->append("FSM -> CLIENT: " + doc.toJson(QJsonDocument::Compact));

    if (msg.contains("type") && msg["type"].toString() == "CURRENT_STATE") {
        if (msg.contains("payload") && msg["payload"].isObject()) {
            QJsonObject payload = msg["payload"].toObject();
            if (payload.contains("name")) {
                QString currentStateName = payload["name"].toString();
                // ui->label_currentState->setText("Current FSM State: " + currentStateName);

                // Example: Highlight the current state node in your QtNodes view
                // You'll need a mapping from state name to NodeId
                // NodeId currentFsmNodeId = graphModel->findNodeByName(currentStateName);
                // if (currentFsmNodeId != QtNodes::InvalidNodeId) {
                //    nodeScene->clearSelection();
                //    nodeScene->nodeItems().value(currentFsmNodeId)->setSelected(true);
                // }
            }
        }
    }
}

/**
 *    QPROCESS SLOTS
 *  ========================================================================
 *  ========================================================================
 */

void MainWindow::onPythonProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qInfo() << "[MainWindow] Python FSM process finished. Exit code:" << exitCode << "Status:" << exitStatus;
    QString stdOut = pythonFsmProcess->readAllStandardOutput();
    QString stdErr = pythonFsmProcess->readAllStandardError();
    if(!stdOut.isEmpty()) ui->textEdit_logOut->append("PYTHON STDOUT (on finish):\n" + stdOut);
    if(!stdErr.isEmpty()) ui->textEdit_logOut->append("PYTHON STDERR (on finish):\n" + stdErr);

    // If client was connected, it will likely disconnect now or soon
    // Update UI to show FSM is not running
    // TODO: add correct label
    //ui->label_currentState->setText("Current FSM State: Not Running");
}

void MainWindow::onPythonProcessError(QProcess::ProcessError error) {
    qWarning() << "[MainWindow] Python FSM process error:" << error << pythonFsmProcess->errorString();
    ui->textEdit_logOut->append("PYTHON PROCESS ERROR: " + pythonFsmProcess->errorString());
    // Update UI
}

void MainWindow::onPythonProcessStateChanged(QProcess::ProcessState newState) {
    qInfo() << "[MainWindow] Python FSM process state changed to:" << newState;
    // You can update UI based on this, e.g., "Starting...", "Running..."
}

void MainWindow::onPythonReadyReadStdOut() {
    QByteArray data = pythonFsmProcess->readAllStandardOutput();
    qInfo() << "[PYTHON STDOUT] " << data.trimmed(); // Log to console/debug output
    ui->textEdit_logOut->append("PYTHON STDOUT: " + QString::fromUtf8(data)); // Also show in UI log
    // You could parse this for "Server ready" messages
}

void MainWindow::onPythonReadyReadStdErr() {
    QByteArray data = pythonFsmProcess->readAllStandardError();
    qWarning() << "[PYTHON STDERR] " << data.trimmed(); // Log to console/debug output
    ui->textEdit_logOut->append("PYTHON STDERR: " + QString::fromUtf8(data)); // Also show in UI log
}


void MainWindow::onFsmClientError(const QString& err) {
    qWarning() << "[MainWindow] FSM Client error:" << err;
    ui->textEdit_logOut->append("CLIENT ERROR: " + err);
    // Update UI
}




/**
 *    NODEEDITOR SIGNALS
 *  ========================================================================
 *  ========================================================================
 */

void MainWindow::onNodeClicked(NodeId const nodeId)
{
    qWarning() << "Node clicked! Id:" << nodeId;

    // Save the node id that was clicked
    lastSelectedNode = nodeId;

    // Enable the code text edit and state name lineedit but disable conn edit
    ui->textEdit_actionCode->setEnabled(true);
    ui->lineEdit_stateName->setEnabled(true);
    ui->textEdit_connCond->setEnabled(false);
    // enable the button for setting start state
    ui->pushButton_setStartState->setEnabled(true);
    // enable checkbox fgor setting final state
    ui->checkBox_isFinal->setEnabled(true);


    // Set the textbox text to the code of clicked node
    auto nodeText = graphModel->GetNodeActionCode(nodeId);
    ui->textEdit_actionCode->setText(nodeText);
    auto nodeName = graphModel->GetNodeName(nodeId);
    ui->lineEdit_stateName->setText(nodeName);
    auto checkboxState = graphModel->GetNodeFinalState(nodeId) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    ui->checkBox_isFinal->setCheckState(checkboxState);

    // disable the Start State button if this state is already set as start
    if(graphModel->IsStartNode(nodeId))
    {
        ui->pushButton_setStartState->setEnabled(false);
        ui->pushButton_setStartState->setText("✅Start State");
    }
    else // otherwise set the normal text
    {
        ui->pushButton_setStartState->setEnabled(true);
        ui->pushButton_setStartState->setText("Set as Start State");
    }
}

void MainWindow::onNodeSelectionChanged()
{
    qWarning() << "Selection Changed!";

    // if there is nothing selected, disable the UI elems fro adjusting state settings
    if(nodeScene->selectedItems().empty())
    {
        ui->textEdit_actionCode->setEnabled(false);
        ui->lineEdit_stateName->setEnabled(false);
        ui->textEdit_connCond->setEnabled(false);
        ui->pushButton_setStartState->setEnabled(false);
        ui->checkBox_isFinal->setEnabled(false);
    }
}

void MainWindow::onConnectionClicked(ConnectionId const connId)
{
    qWarning() << "Connection clicked!";
    // save the conn id
    lastSelectedConnId = connId;

    // while editing the connection id, disable state name line edit
    ui->lineEdit_stateName->setEnabled(false);
    // disable the textEdit for the code editing
    ui->textEdit_actionCode->setEnabled(false);
    // enable the textEdit for the connection condition code:
    ui->textEdit_connCond->setEnabled(true);
    // disable the finalState checkbox and start stateb btn:
    ui->checkBox_isFinal->setEnabled(false);
    ui->pushButton_setStartState->setEnabled(false);

    // add the current connection code:
    auto connCode = graphModel->GetConnectionCode(connId);
    ui->textEdit_connCond->setText(connCode);
}

/**
 *    UI ELEMENTS SINGALS
 *  ========================================================================
 *  ========================================================================
 */

inline std::string trimToStdString(const QString& str) {
    return str.trimmed().toStdString();
}

std::vector<std::pair<std::string, std::string>> parseVariableTextBox(const QString& input) {
    std::vector<std::pair<std::string, std::string>> result;
    QTextStream stream(const_cast<QString*>(&input));  // QTextStream needs non-const QString

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || !line.contains("="))
            continue;

        QStringList parts = line.split("=", Qt::KeepEmptyParts);
        if (parts.size() != 2)
            continue;  // skip malformed lines

        std::string varName = trimToStdString(parts[0]);
        std::string value = trimToStdString(parts[1]);

        result.emplace_back(varName, value);
    }

    return result;
}

void MainWindow::on_button_Run_clicked()
{
    // --- 0. Stop any existing FSM and client ---
    if (pythonFsmProcess->state() != QProcess::NotRunning) {
        qInfo() << "[MainWindow] Stopping existing FSM process...";
        pythonFsmProcess->terminate(); // Or pythonFsmProcess->kill();
        if (!pythonFsmProcess->waitForFinished(3000)) { // Wait 3s
            qWarning() << "[MainWindow] Python FSM process did not terminate gracefully. Forcing kill.";
            pythonFsmProcess->kill();
            pythonFsmProcess->waitForFinished(); // Wait for kill
        }
        qInfo() << "[MainWindow] Previous FSM process stopped.";
    }

    // TODO: implement the isConnected method
    if (fsmClient->isConnected()) {
        qInfo() << "[MainWindow] Disconnecting existing client...";
        fsmClient->disconnectFromServer();
        // Wait for disconnection or just proceed, as new connection will override
    }

    // here update UI to maybe reflect "not running"

    // --- 1. Get Automaton Data ---
    std::unique_ptr<Automaton> automaton(graphModel->ToAutomaton());
    if (!automaton) {
        qWarning() << "[MainWindow] Failed to get automaton data from model.";
        // Show error to user
        return;
    }

    // --- Variables ---
    // get the texbox contents
    auto variablesTextEdit = ui->textEdit_vars->toPlainText();
    // parse the variable definition textbox contents
    auto variables = parseVariableTextBox(variablesTextEdit);

    // add the parsed varaible names and their values to the automaton:
    for (const auto& [varName, value] : variables)
    {
        automaton->addVariable(varName, value);
    }

    // --- 2. Generate Python FSM Code ---

    InterpretGenerator generator;
    QString pythonFilePath = QDir::currentPath() + "/interpret/output.py";
    QDir().mkpath(QFileInfo(pythonFilePath).path()); // Ensure directory exists

    qDebug() << "[MainWindow] Generating Python FSM at:" << pythonFilePath;
    generator.generate(*automaton, pythonFilePath);

    // --- 3. Run the generated Python file ---
    QString pythonExe = "python"; // this could be configurable
    QString logFilePath = QDir::currentPath() + "/interpret/output.log";
    QDir().mkpath(QFileInfo(logFilePath).path()); // Ensure directory exists

    // Clear previous log content
    QFile logFile(logFilePath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        logFile.close();
    }

    pythonFsmProcess->setStandardOutputFile(logFilePath, QIODevice::Append); // Append for multiple runs in one session
    pythonFsmProcess->setStandardErrorFile(logFilePath, QIODevice::Append);

    qInfo() << "[MainWindow] Starting Python FSM server process:" << pythonExe << pythonFilePath;

    pythonFsmProcess->start(pythonExe, QStringList() << pythonFilePath);
    if (!pythonFsmProcess->waitForStarted(5000)) { // 5 sec
        qWarning() << "[MainWindow] Failed to start Python process!";
        qWarning() << "[MainWindow] Python Process Error:" << pythonFsmProcess->errorString();
        qWarning() << "[MainWindow] Stderr:" << pythonFsmProcess->readAllStandardError();
        qWarning() << "[MainWindow] Stdout:" << pythonFsmProcess->readAllStandardOutput();
        // Show error to user
        return;
    }

    qInfo() << "[MainWindow] Python FSM process started successfully.";

    // --- 4. Connect C++ client to the interpret ---
    QTimer::singleShot(2000, this, [this]() { // QTimer means non-blocking delay
        qInfo() << "[MainWindow] Attempting to connect client to FSM server...";
        fsmClient->connectToServer("localhost", 65432);
    });

    // process.waitForFinished(-1); // Wait until finished
    // qDebug() << "Python script finished. Output written to:" << logFilePath;
}

void MainWindow::on_button_addState_clicked()
{
    auto id = graphModel->addNode();
    graphModel->setNodeData(id, NodeRole::Position, QPointF(0, 0));
    graphModel->setNodeData(id, NodeRole::OutPortCount, 1);
    graphModel->setNodeData(id, NodeRole::InPortCount, 1);
}

void MainWindow::on_textEdit_actionCode_textChanged()
{
    // update the code of selected node:
    auto textEditContent = ui->textEdit_actionCode->toPlainText();
    graphModel->SetNodeActionCode(lastSelectedNode, textEditContent);
}

void MainWindow::on_lineEdit_stateName_textChanged(const QString &text)
{
    qWarning() << "Node name update!";
    // update the name of selected node:
    graphModel->SetNodeName(lastSelectedNode, text);
}


void MainWindow::on_textEdit_connCond_textChanged()
{
    auto connCode = ui->textEdit_connCond->toPlainText();
    graphModel->SetConnectionCode(lastSelectedConnId, connCode);
}


void MainWindow::on_checkBox_isFinal_stateChanged(int state)
{
    // state:
    // 0 = not-checked
    // 2 = checked
    auto isChecked = (state == 2);
    graphModel->SetNodeFinalState(lastSelectedNode, isChecked);
}


void MainWindow::on_pushButton_setStartState_clicked()
{
    graphModel->SetStartNode(lastSelectedNode);
    // disable the Start State button if this state is already set as start
    if(graphModel->IsStartNode(lastSelectedNode))
    {
        ui->pushButton_setStartState->setEnabled(false);
        ui->pushButton_setStartState->setText("✅Start State");
    }

}


void MainWindow::on_button_Stop_clicked()
{
    if (fsmClient && fsmClient->isConnected()) { // Check if client exists and is connected
        qInfo() << "[MainWindow] Sending STOP_FSM command to Python interpreter.";
        ui->textEdit_logOut->append("CLIENT -> FSM: Sending STOP_FSM command.");
        fsmClient->sendStopFsm();
        // The Python FSM should then shut down, which will likely cause
        // pythonFsmProcess to finish and the m_fsmClient to disconnect.
        // The onFsmClientDisconnected and onPythonProcessFinished slots will handle UI updates.
    } else {
        qWarning() << "[MainWindow] Cannot send STOP_FSM: Client not connected.";
        ui->textEdit_logOut->append("CLIENT: Cannot send STOP_FSM - not connected.");
    }
}

