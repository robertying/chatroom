#include "stdafx.h"
#include "chatroom.h"
#include <QDialog>
#include <QLabel>
#include <QStatusBar>
#include <QFile>
#include <QTextBrowser>
#include <QTextStream>
#include <QDebug>
mutex mtx;
mutex mtx2;

int Client::InitializeClient(MainWindow* mainWindow)
{
    //call dll
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
    mainWindow->statusBar()->showMessage(QString::fromLocal8Bit("客户端初始化..."));
}

bool Client::ConnectClientSocket(MainWindow* mainWindow)
{
    //connect
    bool success=connect(sockClient, (SOCKADDR *)&addrServ, sizeof(SOCKADDR));
    Online = true;
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
    int status=send(sockClient, StringToSend, int(strlen(StringToSend)), 0);
    if (status <= 0)
    {
        CloseClientSocket(mainWindow);
        ConnectionLost();
    }
}

void User::ReceiveString(char * StringToReceive)
{
    recv(sockClient, StringToReceive, 200, 0);
}

void User::Input()
{
    char inputBuffer[200];
    memset(inputBuffer, 0, sizeof(inputBuffer));
    if (Online)
    {
        //acquire messages
        string temp=Message.toStdString();



        //acquire local time
        time_t tTime = time(NULL);

        //compose whole log
        strstream str(inputBuffer, 200);
        str << Name << " " << tTime << " " << temp<<'\n';

        //send
        mtx2.lock();
        SendString(inputBuffer);
        mtx2.unlock();
        memset(inputBuffer, 0, sizeof(inputBuffer));
    }
    else
    {
        qDebug()<<"im here";
    //quit option
       // Sleep(500); //TODO CONSIDER PROPER TIME WHEN QUIT

        //last few things to attend to
        time_t tTime = time(NULL);
        strstream str(inputBuffer, 200);
        str << Name << " " << tTime << " " << "quit\n";
        mtx2.lock();
        SendString(inputBuffer);
        mtx2.unlock();
        memset(inputBuffer, 0, sizeof(inputBuffer));

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
    while (Online)
    {
        //receive
        ReceiveString(recvBuffer);
        fstream output;
        output.open(UtfName+"_log.txt", ios::app | ios::out);
        output << recvBuffer;
        output.close();

/*		//convert char to log
        char name[20] = { '\0' };
        char ctime[20] = { '\0' };
        char content[100] = { '\0' };
        time_t time;

        string str = string(recvBuffer);
        int begin = str.find_first_of(' ');
        int end = str.find(' ', begin + 1);
        str.copy(name, begin, 0);
        str.copy(ctime, end - begin, begin);
        time = atol(ctime);
        string newstr = str.erase(0, end + 1);
        newstr.copy(content, newstr.length(), 0);

        ClientLog temp;
        temp.SetLog(string(name), time, content);

        //save to client log
        fstream output;
        output << temp;
*/
        memset(recvBuffer, 0, sizeof(recvBuffer));
    }
}

void User::ReadFile(QTextBrowser* textBrowser)
{
    while (Online)
    {
    QFile file(QString::fromStdString(UtfName+"_log.txt"));
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);
    textBrowser->setText(in.readAll());
    file.close();
    }
}

void User::ConnectionLost()
{
    //.....
    Reconnect();
}

void User::Reconnect()
{
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
    return LogContent.rawTime;
}
