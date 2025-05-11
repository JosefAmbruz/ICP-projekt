
/**
 * @file main.cpp
 * @brief Entry point for the application.
 * 
 * This file contains the main function, which initializes the QApplication
 * and displays the main window of the application.
 * 
 * @author Josef Ambruz, Jakub Kovařík
 * @date 2025-5-11
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
