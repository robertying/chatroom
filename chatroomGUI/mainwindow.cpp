#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "logindialog.h"
#include <chatroom.h>
#include <QLabel>
#include <QKeyEvent>
#include <QTimer>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QString>
#include <QFileSystemWatcher>
#include <QDesktopServices>
#include <QDesktopWidget>

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

    //set name as a permanent label in status bar
    QLabel* perLabel=new QLabel(QString::fromStdString(user.ShowName())+QString::fromLocal8Bit("已在线"),this);
    ui->statusBar->addPermanentWidget(perLabel);

    //client start after all the window is shown. this can prevent frozen window
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(1);
    connect(timer, SIGNAL(timeout()), this, SLOT(ClientStart()));
    
    //watch file change, if changed then begin to readfile and present on the window
    QFileSystemWatcher* watcher = new QFileSystemWatcher(this);
    watcher->addPath(QString::fromStdString(user.ShowName()+"_log.txt"));
    connect(watcher,SIGNAL(fileChanged(QString)),SLOT(ReadFile()));

    //put window in the middle of the screen
    QDesktopWidget* desktop = QApplication::desktop();
    move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);

    //prevent readfile start before the window is shown
    mtx2.lock();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_about_triggered()
{
    //show about window
    About* about=new About;
    about->show();
}

void MainWindow::on_pushButton_released()
{
    //send messages to the input thread
    user.Message = ui->textEdit->toPlainText();

    //after message is set, then begin the thread
    mtx2.unlock();

    ui->textEdit->clear();
    std::thread InputThread(&User::Input, user);
    InputThread.join();

    mtx2.lock();
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    //if press enter then call the button released function
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

    //only read file when triggered by file changed signal
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
            //translate quit to formal message
            ui->textBrowser->append(QString::fromStdString(string(name)+"  @"+string(dt))+QString::fromLocal8Bit("已退出聊天室")+"\n");
        }
        else
        {
            //put message logs on the window
            ui->textBrowser->append(QString::fromStdString(string(name)+"  @"+string(dt)+string(content)+"\n"));

            //if received a new message then alert the window
            QApplication::alert(this);
        }
        memset(log, 0, sizeof(log));
    }
}

void MainWindow::ClientStart()
{
   user.InitializeClient(this);
   user.CreateClientSocket(this);

   //check if connection is alive with server
   if (!user.ConnectClientSocket(this))
   {
        //show status
        ui->statusBar->showMessage(QString::fromLocal8Bit("无法连接到服务器..."));

        //set message box
        QMessageBox msgBox(QMessageBox::Warning, QString::fromLocal8Bit("错误"),
                           QString::fromLocal8Bit("无法连接到服务器，是否重试？"),
                           QMessageBox::Yes | QMessageBox::No);
        msgBox.setWindowIcon(QIcon(":/image/icon.png"));
        msgBox.setButtonText(QMessageBox::Yes,QString::fromLocal8Bit("重试"));
        msgBox.setButtonText(QMessageBox::No,QString::fromLocal8Bit("退出"));

        if (msgBox.exec() == QMessageBox::No)
        {
            //close the program
            user.CloseClientSocket(this);
            close();
        }
        else
        {
            ClientStart();
        }
    }

    //recreate log to begin a new client
    fstream output;
    output.open(QString::fromStdString(user.ShowName()).toLocal8Bit().toStdString()+"_log.txt", ios::out);
    output.close();

    //begin to receive messages from server
    std::thread OutputThread(&User::Output, user);

    //detach the thread to run after the mainwindow is shut down
    OutputThread.detach();
}

void MainWindow::on_textEdit_textChanged()
{
    //used to limit the text length
    if (ui->textEdit->toPlainText().length() > 80)
    {
        QString text = ui->textEdit->toPlainText();

        //cut off at 80 characters
        text.chop(text.length() - 80);
        //reset text
        ui->textEdit->setPlainText(text);

        //reset cursor
        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.setPosition(ui->textEdit->document()->characterCount() - 1);
        ui->textEdit->setTextCursor(cursor);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    //quit confirmation and close necessary things
    //set message box
    QMessageBox msgBox(QMessageBox::Question, QString::fromLocal8Bit("退出程序"),
                       QString::fromLocal8Bit("确认退出程序？"),
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowIcon(QIcon(":/image/icon.png"));
    msgBox.setButtonText(QMessageBox::Yes,QString::fromLocal8Bit("确定"));
    msgBox.setButtonText(QMessageBox::No,QString::fromLocal8Bit("取消"));

    if (msgBox.exec() == QMessageBox::No)
    {
        //ignore the close signal and carry on
        event->ignore();
    }
    else
    {
        //same as input thread
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

        //close the program
        event->accept();
    }
}

void MainWindow::on_action_L_triggered()
{
    //open local log when button triggered
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(user.ShowName()+"_log.txt")));
}
