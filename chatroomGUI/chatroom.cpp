#include "chatroom.h"
#include <QFile>
#include <QTextBrowser>
#include <QTextStream>
#include <QStatusBar>

//used for file locks
mutex mtx;
mutex mtx2;

int Client::InitializeClient(MainWindow* mainWindow)
{
    //call Windows socket dll
    int err;
    WSADATA wsaData;
    WORD wVersionRequested;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) return -1;
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();
        return -2;
    }

    //show status in mainwindow
    mainWindow->statusBar()->showMessage(QString::fromLocal8Bit("客户端初始化..."));
    return 0;
}

void Client::CreateClientSocket(MainWindow* mainWindow)
{
    //create socket
    sockClient = socket(AF_INET, SOCK_STREAM, 0);

    //set server address
    addrServ.sin_addr.S_un.S_addr = inet_addr("45.55.3.170");
    addrServ.sin_family = AF_INET;
    addrServ.sin_port = htons(42001);

    //show status in mainwindow
    mainWindow->statusBar()->showMessage(QString::fromLocal8Bit("客户端初始化..."));
}

bool Client::ConnectClientSocket(MainWindow* mainWindow)
{
    //connect
    bool success=connect(sockClient, (SOCKADDR *)&addrServ, sizeof(SOCKADDR));
    Online = true;

    //show status in mainwindow
    mainWindow->statusBar()->showMessage(QString::fromLocal8Bit("客户端已连接..."));
    return (!success);
}

void Client::CloseClientSocket(MainWindow* mainWindow)
{
    //close and clean
    mainWindow->statusBar()->showMessage(QString::fromLocal8Bit("关闭客户端连接..."));
    closesocket(sockClient);
    Online = false;
    WSACleanup();
}

User::User()
{
    Name = "Unnamed User";
    Online = false;
}

void User::SetName(string name)
{
    Name = name;
}

void User::SetUtfName(string name)
{
    //to use Unicode for codec
    UtfName=QString::fromStdString(name).toLocal8Bit().toStdString();
}

string User::ShowName()
{
    return Name;
}

string User::ShowUtfName()
{
    return UtfName;
}

void User::SendString(char* StringToSend)
{
    //send messages on socket
    int status=send(sockClient, StringToSend, int(strlen(StringToSend)), 0);

    //check if connection is alive
    if (status <= 0)
    {
        CloseClientSocket(mainWindow);
    }
}

void User::ReceiveString(char * StringToReceive)
{
    //receive messages
    recv(sockClient, StringToReceive, 200, 0);
}

void User::Input()
{
    char inputBuffer[200];
    memset(inputBuffer, 0, sizeof(inputBuffer));

    //if online then keep working
    if (Online)
    {
        //acquire messages from mainwindow
        string temp=Message.toStdString();

        //acquire local time to generate message logs
        time_t tTime = time(NULL);

        //compose whole logs
        strstream str(inputBuffer, 200);
        str << Name << " " << tTime << " " << temp<<'\n';

        //send
        //use lock to avoid the message being changed by another thread
        mtx2.lock();
        SendString(inputBuffer);
        mtx2.unlock();
        memset(inputBuffer, 0, sizeof(inputBuffer));
    }
    else
    {
        //last few things to attend to
        time_t tTime = time(NULL);
        strstream str(inputBuffer, 200);
        str << Name << " " << tTime << " " << "quit\n";

        //send quit to server
        mtx2.lock();
        SendString(inputBuffer);
        mtx2.unlock();
        memset(inputBuffer, 0, sizeof(inputBuffer));

        //locally write the log since offline
        fstream Output;
        Output.open(UtfName+"_log.txt", ios::app | ios::out);
        Output << inputBuffer;
        Output.close();
    }
}

void User::Output()
{
    char recvBuffer[200];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    //if online then keep working
    while (Online)
    {
        //receive
        ReceiveString(recvBuffer);

        //write messages from server to local logs
        fstream output;
        output.open(UtfName+"_log.txt", ios::app | ios::out);
        output << recvBuffer;
        output.close();
        memset(recvBuffer, 0, sizeof(recvBuffer));
    }
}

void User::ReadFile(QTextBrowser* textBrowser)
{
    //if online then keep working
    while (Online)
    {
        //read local log
        QFile file(QString::fromStdString(UtfName+"_log.txt"));
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);

        //present all logs in text browser
        textBrowser->setText(in.readAll());
        file.close();
    }
}

File::File()
{
    strcpy(Path,"log.txt");
}

void File::SetPath(char* path)
{
    Path = path;
}

fstream& operator<<(fstream& output, ClientLog& log)
{
    //file lock
    mtx.lock();

    //read from log file
    output.open(log.Path, ios::in);
    vector <Log> alltemp;
    Log temp;
    output >> temp.Name >> temp.rawTime >> temp.Content;
    while (!output.eof())
    {
        alltemp.push_back(temp);
        output >> temp.Name >> temp.rawTime >> temp.Content;
    }
    output.close();
    mtx.unlock();

    //process log
    bool flag = 0;
    if (alltemp.size() == 0)
    {
        alltemp.insert(alltemp.begin(), log.LogContent);
        flag = 1;
    }

    //put logs in time order
    int i;
    for (i = 0; i < alltemp.size(); ++i)
    {
        if (log.ShowTime() < alltemp[i].rawTime)
        {
            alltemp.insert(alltemp.begin()+i, log.LogContent);
            flag = 1;
            break;
        }
    }

    if (!flag) alltemp.push_back(log.LogContent);

    mtx.lock();
    //write new logs to the log file
    output.open(log.Path, ios::out|ios::app);
    for (i = 0; i < alltemp.size(); ++i)
    {
        output << alltemp[i].Name << " "
            << alltemp[i].rawTime << " "
            << alltemp[i].Content << endl;
    }
    output.close();

    mtx.unlock();
    return output;
}

fstream& operator>>(fstream& input, ClientLog& log)
{
    mtx.lock();

    input.open(log.Path, ios::in);
    input >> log.LogContent.Name >> log.LogContent.rawTime >> log.LogContent.Content;
    input.close();

    mtx.unlock();
    return input;
}

void ClientLog::SetLog(string name, time_t time, char* content)
{
    LogContent.Name = name;
    LogContent.rawTime = time;
    strcpy_s(LogContent.Content, content);
}

time_t ClientLog::ShowTime()
{
    //show raw time
    return LogContent.rawTime;
}
