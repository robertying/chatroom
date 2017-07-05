#ifndef CHATROOM_H
#define CHATROOM_H
#pragma once
#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#include <ctime>
#include <strstream>
#pragma comment (lib,"ws2_32.lib")
#include "mainwindow.h"
#include <QString>
#include <QLabel>
#include <QTextBrowser>
using namespace std;

extern mutex mtx; //used for file locks
extern mutex mtx2;

//log format
struct Log
{
    string Name;
    time_t rawTime;
    char Content[100];
};

//create local client socket
class Client
{
public:
    int InitializeClient(MainWindow* mainWindow);
    void CreateClientSocket(MainWindow* mainWindow);
    bool ConnectClientSocket(MainWindow* mainWindow);
    void CloseClientSocket(MainWindow* mainWindow);

    //online status
    bool Online;
protected:
    SOCKET sockClient;
    SOCKADDR_IN addrServ;
};

//base class for sending and receiving functions
class Communication
{
public:
    virtual void SendString(char* StringToSend) = 0;
    virtual void ReceiveString(char* StringToReceive) = 0;
};

//local user behaviors
class User :public Client,public Communication
{
public:
    User();

    //return true for the valid name, false otherwise
    void SetName(string name);
    void SetUtfName(string name);

    //show name
    string ShowUtfName();
    string ShowName();

    //instantiation for virtual functions
    void SendString(char* StringToSend);
    void ReceiveString(char* StringToReceive);

    //thread functions
    void Input();
    void Output();
    void ReadFile(QTextBrowser* textBrowser);

    //connection
    void ConnectionLost();
    void Reconnect();

    MainWindow* mainWindow;
    QString Message;
    QString MessageToReceive;
protected:
    string Name;
    string UtfName;


};

//base class for io
class File
{
public:
    File();
    void SetPath(char* path);
protected:
    char* Path;
};

//io behaviors for local user
class ClientLog :public File
{
public:
    //overloaded for simplier io
    friend fstream& operator<<(fstream&, ClientLog&);
    friend fstream& operator>>(fstream&, ClientLog&);

    void SetLog(string name, time_t time, char *content);
    time_t ShowTime();
    string tTimeTosTime();
protected:
    //local log
    Log LogContent;
};

#endif // CHATROOM_H
