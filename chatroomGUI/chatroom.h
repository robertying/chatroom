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
#include "mainwindow.h"
#include <QString>
#include <QTextBrowser>
#pragma comment (lib,"ws2_32.lib")
using namespace std;

//used for file locks
extern mutex mtx;
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

//abstract base class for sending and receiving functions
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

    //set name with different codecs
    void SetName(string name);
    void SetUtfName(string name);

    //show name
    string ShowUtfName();
    string ShowName();

    //instantiation of virtual functions
    void SendString(char* StringToSend);
    void ReceiveString(char* StringToReceive);

    //thread functions
    void Input();
    void Output();
    void ReadFile(QTextBrowser* textBrowser);

    //variables to transfer messages between windows and threads
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

    //set messages to send with proper format
    void SetLog(string name, time_t time, char *content);

    //acquire time
    time_t ShowTime();

protected:
    //local log
    Log LogContent;
};

#endif // CHATROOM_H
