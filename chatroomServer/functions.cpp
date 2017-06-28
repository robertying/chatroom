#include "chatroom.h"
int Client::ClientNum = 0;

Client::Client()
{
	Online = false;
}

Server::Server()
{
	Online = false;
}

void Server::CreateServerSocket()
{
	//create server socket
	sockServer = socket(AF_INET, SOCK_STREAM, 0);

	//set server address
	addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
	addrServer.sin_port = htons(42001);
	addrServer.sin_family = AF_INET;
}

void Server::ConnectServerSocket()
{
	//bind the port
	bind(sockServer, (sockaddr *)&addrServer, sizeof(addrServer));
	listen(sockServer, 5);
	cout << "Server started..." << endl;
}

void Server::CloseServerSocket()
{
	close(sockServer);
}

void Admin::CloseClientSocket(int ID)
{
	close(ClientList[ID].sockClient);
}

void Admin::SendString(int ID, char* StringToSend)
{
	send(ClientList[ID].sockClient, StringToSend, strlen(StringToSend), 0);
}

void Admin::ReceiveString(int ID, char* StringToReceive)
{
	recv(ClientList[ID].sockClient, StringToReceive, 200, 0);
}

void Admin::Input(int ID)
{
	char recvBuffer[200];
	memset(recvBuffer, 0, sizeof(recvBuffer));
	while (ClientList[ID].Online)
	{
		//receive
		ReceiveString(ID, recvBuffer);

		//convert char to log
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

		ServerLog temp;
		temp.SetLog(string(name), time, content);

		//save to server log
		fstream output;
		output << temp;

		memset(recvBuffer, 0, sizeof(recvBuffer));
	}
}

void Admin::Output(int ID)
{
	char sendBuffer[200];
	memset(sendBuffer, 0, sizeof(sendBuffer));
	
	ifstream input;
	input.open("log.txt", ios::in);
	streampos p=0;
	while (ClientList[ID].Online)
	{
		//read from log
		input.seekg(p);
		input.getline(sendBuffer, 200);
		p=input.tellg();
		if (input.eof()) continue;

		//send
		SendString(ID, sendBuffer);
		memset(sendBuffer, 0, sizeof(sendBuffer));
	}
}

void Admin::ReceiveRequest()
{
	while (Online) //server is online
	{
		//accept
		socklen_t addrClientSize = sizeof(ClientList[Client::ClientNum].addrClient);
		ClientList[Client::ClientNum].sockClient = accept(sockServer, (struct sockaddr*)&ClientList[Client::ClientNum].addrClient, &addrClientSize);

		//client is online
		printf("User IP:%s connected..\n", inet_ntoa(ClientList[Client::ClientNum].addrClient.sin_addr));
		ClientList[Client::ClientNum].Online = true;

		Client::ClientNum++;
	}
}

File::File()
{
	Path = "log.txt";
}

void File::SetPath(char* path)
{
	Path = path;
}

fstream& operator<<(fstream& output, ServerLog& log)
{
	//file lock
	mtx.lock();

	//read from log file
	output.open(log.Path, ios::in | ios::app);
	vector <Log> alltemp;
	Log temp;
	output >> temp.Name >> temp.rawTime >> temp.Content;
	while (!output.eof())
	{
		alltemp.push_back(temp);
		output >> temp.Name >> temp.rawTime >> temp.Content;
	}
	output.close();

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
			alltemp.insert(alltemp.begin() + i, log.LogContent);
			flag = 1;
			break;
		}
	}

	if (!flag) alltemp.push_back(log.LogContent);

	//write new logs to the log file
	output.open(log.Path, ios::out);
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

fstream& operator>>(fstream& input, ServerLog& log)
{
	mtx.lock();

	input.open(log.Path, ios::in);
	input >> log.LogContent.Name >> log.LogContent.rawTime >> log.LogContent.Content;

	input.close();

	mtx.unlock();
	return input;
}

void ServerLog::SetLog(string name, time_t time, char* content)
{
	LogContent.Name = name;
	LogContent.rawTime = time;
	strcpy(LogContent.Content, content);
}

time_t ServerLog::ShowTime()
{
	return LogContent.rawTime;
}