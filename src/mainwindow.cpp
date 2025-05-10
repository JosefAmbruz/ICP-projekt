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


#include <QApplication>
#include "DynamicPortsModel.hpp"
#include "interpret_generator.h"
#include "spec_parser/automaton-data.hpp"

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
{
    ui->setupUi(this);
    initNodeCanvas();

    connect(nodeScene, &BasicGraphicsScene::nodeClicked, this, &MainWindow::onNodeClicked);
    connect(nodeScene, &BasicGraphicsScene::selectionChanged, this, &MainWindow::onNodeSelectionChanged);
    connect(nodeScene, &BasicGraphicsScene::connectionClicked, this, & MainWindow::onConnectionClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::on_button_Run_clicked()
{
    Automaton* automaton = graphModel->ToAutomaton();

    InterpretGenerator generator;
    QString file_path = QDir::currentPath() + "/interpret/output.py";

    qDebug() << "File path:" << file_path;
    generator.generate(*automaton, file_path);
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

