#include "mainwindow.h"
#include "logindialog.h"
#include "chatroom.h"
#include <QApplication>

User user;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //login with name
    LoginDialog login;
    if (login.exec()==QDialog::Accepted)
    {
        MainWindow w;
        user.mainWindow=&w;

        w.show();
        return a.exec();
    }
    else return 0;
}
