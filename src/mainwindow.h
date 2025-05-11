#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNodes/BasicGraphicsScene>
#include <QProcess>
#include <QTimer>
#include "DynamicPortsModel.hpp"
#include "client.hpp"


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
    void onSaveToFileClicked();
private slots:
    void on_button_addState_clicked();

    void on_button_Run_clicked();

    void on_textEdit_actionCode_textChanged();

    void on_lineEdit_stateName_textChanged(const QString &arg1);

    void on_textEdit_connCond_textChanged();

    void on_checkBox_isFinal_stateChanged(int arg1);

    void on_pushButton_setStartState_clicked();

    void on_lineEdit_fsmName_textChanged(const QString &arg1);

    // Slots for FSM Client
    void onFsmClientConnected();

    void onFsmClientDisconnected();

    void onFsmClientMessageReceived(const QJsonObject& msg);

    void onFsmClientError(const QString& err);

    // Slots for Python Process
    void onPythonProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onPythonProcessError(QProcess::ProcessError error);

    void onPythonProcessStateChanged(QProcess::ProcessState newState);

    void onPythonReadyReadStdOut();

    void onPythonReadyReadStdErr();

    void on_button_Stop_clicked();

private:
    Ui::MainWindow *ui;
    void initNodeCanvas();
    void initializeModel();
    DynamicPortsModel* graphModel;
    QtNodes::BasicGraphicsScene* nodeScene;

    NodeId lastSelectedNode;
    ConnectionId lastSelectedConnId;

    // Added client and interpret as member variables
    // the logic is that this way, the lifetime of the
    // client and interpret process will be tied to the
    // main window
    FsmClient* fsmClient;
    QProcess* pythonFsmProcess;

    QString automatonName; // member variable intended to be initialized when the spec
                           // is loaded from the source file
    QString automatonDescription;
};
#endif // MAINWINDOW_H
