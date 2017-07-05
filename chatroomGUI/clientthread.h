#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H
#include <QThread>
#include <QLabel>
#include "mainwindow.h"

class ClientThread:public QThread
{
    Q_OBJECT

public:
    ClientThread(MainWindow* w):mainWindow(w)
    {}
protected:
    void run();

private:
    MainWindow* mainWindow;
};


#endif // CLIENTTHREAD_H
