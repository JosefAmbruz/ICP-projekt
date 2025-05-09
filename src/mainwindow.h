#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNodes/BasicGraphicsScene>
#include "DynamicPortsModel.hpp"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onNodeClicked(NodeId const nodeId);
    void onNodeSelectionChanged();
    void onConnectionClicked(ConnectionId connId);
private slots:
    void on_button_addState_clicked();

    void on_button_Run_clicked();

    void on_textEdit_actionCode_textChanged();

    void on_lineEdit_stateName_textChanged(const QString &arg1);

    void on_textEdit_connCond_textChanged();

private:
    Ui::MainWindow *ui;
    void initNodeCanvas();
    void initializeModel();
    DynamicPortsModel* graphModel;
    QtNodes::BasicGraphicsScene* nodeScene;

    NodeId lastSelectedNode;
    ConnectionId lastSelectedConnId;
};
#endif // MAINWINDOW_H
