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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_button_addState_clicked()
{
    auto id = graphModel->addNode();
    graphModel->setNodeData(id, NodeRole::Position, QPointF(0, 0));
    graphModel->setNodeData(id, NodeRole::OutPortCount, 1);
    graphModel->setNodeData(id, NodeRole::InPortCount, 1);
}


void MainWindow::on_button_Run_clicked()
{
    Automaton automaton;

    InterpretGenerator generator;
    QString file_path = QDir::currentPath() + "/interpret/output.py";

    qDebug() << "File path:" << file_path;
    generator.generate(automaton, file_path);

}

