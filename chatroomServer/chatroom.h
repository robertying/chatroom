#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <vector>
using namespace std;

extern pthread_mutex_t mtx; //used for file locks

//used to pass parameters
class Admin;
struct Parameter
{
	Admin* pThis;
	int ID;
};

//log format
struct Log
{
	string Name;
	time_t rawTime;
	char Content[100];
};

//save client sockets
class Client
{
	friend class Admin;
public:
	Client();
	static int ClientNum;
	bool Online;
protected:
	int sockClient;
	sockaddr_in addrClient;
};

//create server socket
class Server
{
public:
	Server();
	void CreateServerSocket();
	void ConnectServerSocket();
	void CloseServerSocket();
	bool Online;
protected:
	int sockServer;
	sockaddr_in addrServer;
};

//base class for sending and receiving functions
class Communication
{
public:
	virtual void SendString(char* StringToSend);
	virtual void ReceiveString(char* StringToReceive);
};

//server behaviors
class Admin :public Server, public Communication
{
public:
	//instantiation for virtual functions
	void SendString(int ID, char* StringToSend);
	void ReceiveString(int ID, char* StringToReceive);

	//receive client requests
	static void ReceiveRequest(void* args);

	//close client sockets
	void CloseClientSocket(int ID);

	//thread functions
	static void Input(void* args);
	static void Output(void* args);
protected:
	//for increasing clients
	vector <Client> ClientList;
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

//io behaviors for server
class ServerLog :public File
{
public:
	//overloaded for simplier io
	friend fstream& operator<<(fstream&, ServerLog&);
	friend fstream& operator>>(fstream&, ServerLog&);

	void SetLog(string name, time_t time, char *content);
	time_t ShowTime();
protected:
	//server log
	Log LogContent;
};