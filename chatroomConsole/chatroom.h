#pragma once
#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#pragma comment (lib,"ws2_32.lib")
using namespace std;

extern mutex mtx; //used for file locks

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
	int InitializeClient();
	void CreateClientSocket();
	void ConnectClientSocket();
	void CloseClientSocket();
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
	bool SetName(string name);

	//instantiation for virtual functions
	void SendString(char* StringToSend); 
	void ReceiveString(char* StringToReceive);

	//thread functions
	void Input();
	void Output();

	//online status
	bool Online;
protected:
	string Name;
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
protected:
	//local log
	Log LogContent;
};