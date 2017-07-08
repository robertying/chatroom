#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>
#include "chatroom.h"

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
        //create mainwindow




        //initialize client socket
//        void ClientStart(MainWindow*);
//        thread ClientStartThread(&ClientStart,&w);

       // string input=
        //threads to input and output logs
/*        thread OutputThread(&User::Output, user);
        thread InputThread(&User::Input, user);
        InputThread.join();
        OutputThread.join();
*/
        //close client socket
//        ClientStartThread.join();
//        user.CloseClientSocket(&w);

        return a.exec();
    }
    else return 0;
}
