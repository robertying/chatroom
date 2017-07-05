#include "stdafx.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "logindialog.h"
#include <QLabel>
#include <chatroom.h>
#include <QKeyEvent>
#include <QTimer>
#include <QMessageBox>
#include <QPushButton>
#include "clientthread.h"
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QDesktopServices>
extern User user;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    createBool=false;
    //recreate log
        fstream output;
        output.open(user.ShowUtfName()+"_log.txt", ios::out);
        output.close();

    ui->setupUi(this);
    ui->textEdit->installEventFilter(this);

    QLabel* perLabel=new QLabel(QString::fromStdString(user.ShowName())+QString::fromLocal8Bit("已在线"),this);
    ui->statusBar->addPermanentWidget(perLabel);

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(1);
    connect(timer, SIGNAL(timeout()), this, SLOT(ClientStart()));
    
    QFileSystemWatcher* watcher = new QFileSystemWatcher(this);
    watcher->addPath(QString::fromStdString(user.ShowName()+"_log.txt"));
    connect(watcher,SIGNAL(fileChanged(QString)),SLOT(ReadFile()));

    
    mtx2.lock();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_about_triggered()
{
    About* about=new About;
    about->show();
}


void MainWindow::on_pushButton_released()
{
    user.Message = ui->textEdit->toPlainText();
    mtx2.unlock();
    ui->textEdit->clear();
    std::thread InputThread(&User::Input, user);
    InputThread.join();
    mtx2.lock();
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->textEdit)
    {
        if(event->type() == QEvent::KeyPress)
        {
             QKeyEvent *k = static_cast<QKeyEvent *>(event);
             if(k->key() == Qt::Key_Return)
             {
                 on_pushButton_released();
                 return true;
             }
        }
    }
    return QWidget::eventFilter(target,event);
}

void MainWindow::ReadFile()
{


    char log[200];
    memset(log, 0, sizeof(log));

        ifstream input;
        streampos p=0;
        ui->textBrowser->clear();
    while (1)
    {
            input.open(user.ShowUtfName()+"_log.txt", ios::in);
            input.seekg(p);
            input.getline(log, 200);
            if (strlen(log) == 0)
            {
                input.close();

                break;
            }

 //           if (string(log)=="Chatroom created..."&&!createBool)
 //           {
 //               ui->textBrowser->append(QString::fromLocal8Bit("聊天室已创建..."));
 //               createBool=true;
 //               break;
 //           }

            p = input.tellg();
            input.close();

            //convert char to log
                    char name[20] = { '\0' };
                    char cTime[20] = { '\0' };
                    char content[100] = { '\0' };
                    time_t time;

                    string str = string(log);
                    int begin = int(str.find_first_of(' '));
                    int end = int(str.find(' ', begin + 1));
                    str.copy(name, begin, 0);

                    str.copy(cTime, end - begin-1, begin+1);

                    time=atol(cTime);
                    char* dt = ctime(&time);

                    string newstr = str.erase(0, end + 1);
                    newstr.copy(content, newstr.length(), 0);
                    if (newstr=="quit")
                    {
                        ui->textBrowser->append(QString::fromStdString(string(name)+"  @"+string(dt))+QString::fromLocal8Bit("已退出聊天室")+"\n");
                    }
                    else
                    {
                    ui->textBrowser->append(QString::fromStdString(string(name)+"  @"+string(dt)+string(content)+"\n"));
                }
                    memset(log, 0, sizeof(log));


}

}

void MainWindow::ClientStart()
{
   user.InitializeClient(this);



    user.CreateClientSocket(this);
   if (!user.ConnectClientSocket(this))
   {
        ui->statusBar->showMessage(QString::fromLocal8Bit("无法连接到服务器..."));

       QMessageBox msgBox(QMessageBox::Warning, QString::fromLocal8Bit("错误"),
               QString::fromLocal8Bit("无法连接到服务器，是否重试？"),
               QMessageBox::Yes | QMessageBox::No);
          msgBox.setWindowIcon(QIcon(":/image/icon.png"));
          msgBox.setButtonText(QMessageBox::Yes,QString::fromLocal8Bit("重试"));
          msgBox.setButtonText(QMessageBox::No,QString::fromLocal8Bit("退出"));

        if (msgBox.exec() == QMessageBox::No)
        {
           user.CloseClientSocket(this);
close();
        }
        else
        {
ClientStart();

        }
    }

    //recreate log
    fstream output;
    output.open(QString::fromStdString(user.ShowName()).toLocal8Bit().toStdString()+"_log.txt", ios::out);
    output.close();


    std::thread OutputThread(&User::Output, user);
    OutputThread.detach();

}

void MainWindow::on_textEdit_textChanged()
{
    if (ui->textEdit->toPlainText().length() > 80)
    {
        QString text = ui->textEdit->toPlainText();
        text.chop(text.length() - 80); // Cut off at 300 characters
        ui->textEdit->setPlainText(text); // Reset text

        // This code just resets the cursor back to the end position
        // If you don't use this, it moves back to the beginning.
        // This is helpful for really long text edits where you might
        // lose your place.
        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.setPosition(ui->textEdit->document()->characterCount() - 1);
        ui->textEdit->setTextCursor(cursor);

    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{



            qDebug()<<user.Online;
    QMessageBox msgBox(QMessageBox::Question, QString::fromLocal8Bit("退出程序"),
            QString::fromLocal8Bit("确认退出程序？"),
            QMessageBox::Yes | QMessageBox::No);
       msgBox.setWindowIcon(QIcon(":/image/icon.png"));
       msgBox.setButtonText(QMessageBox::Yes,QString::fromLocal8Bit("确定"));
       msgBox.setButtonText(QMessageBox::No,QString::fromLocal8Bit("取消"));

        if (msgBox.exec() == QMessageBox::No)
        {
            event->ignore();  //忽略退出信号，程序继续运行

        }
        else
        {

            mtx2.unlock();

            //last few things to attend to
            char inputBuffer[200];
            time_t tTime = time(NULL);
            strstream str(inputBuffer, 200);
            str << user.ShowName() << " " << tTime << " " << "quit\n";

            user.SendString(inputBuffer);

            user.CloseClientSocket(this);

            fstream Output;
            Output.open(user.ShowUtfName()+"_log.txt", ios::app | ios::out);
            Output << inputBuffer;
            Output.close();

            event->accept();  //接受退出信号，程序退出
        }
}

void MainWindow::on_action_L_triggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(user.ShowName()+"_log.txt")));
}
