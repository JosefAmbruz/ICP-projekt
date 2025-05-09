#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    // new thread
    // Client.Run(w.ui)
    //     while () { data = socket.Read (); }

    return a.exec();
}
