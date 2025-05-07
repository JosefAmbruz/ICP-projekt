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

private slots:
    void on_button_addState_clicked();

private:
    Ui::MainWindow *ui;
    void initNodeCanvas();
    void initializeModel();
    DynamicPortsModel* graphModel;
    QtNodes::BasicGraphicsScene* nodeScene;
};
#endif // MAINWINDOW_H
