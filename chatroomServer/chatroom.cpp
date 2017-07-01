#include "chatroom.h"
int Client::ClientNum = 0;
vector <pthread_t> ReceiveThread;
vector <pthread_t> SendThread;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

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
	cout << "CreateServerSocket" << endl;
}

void Server::ConnectServerSocket()
{
	//bind the port
	bind(sockServer, (sockaddr *)&addrServer, sizeof(addrServer));
	listen(sockServer, 5);
	Online = true;
	cout << "Server started..." << endl;
}

void Server::CloseServerSocket()
{
	close(sockServer);
	cout << "CloseServerSocket" << endl;
	Online = false;
}

void Admin::CloseClientSocket(int ID)
{
	close(ClientList[ID].sockClient);
	cout << "Client " << ID << " closed" << endl;
	ClientList[ID].Online = false;
}

void Admin::SendString(int ID, char* StringToSend)
{
	int status;
	status=send(ClientList[ID].sockClient, StringToSend, strlen(StringToSend), 0);
	if (status < 0)
	{
		CloseClientSocket(ID);
	}
	cout << "SendString" << endl;
}

void Admin::ReceiveString(int ID, char* StringToReceive)
{
	recv(ClientList[ID].sockClient, StringToReceive, 200, 0);
	cout << "ReceiveString" << endl;
}

void* Admin::Input(void* args)
{
	cout << "Call Input Thread" << endl;
	Parameter *para = (Parameter*)args;
	char recvBuffer[200];
	memset(recvBuffer, 0, sizeof(recvBuffer));
	while (para->pThis->ClientList[para->ID].Online)
	{
		//receive
		para->pThis->ReceiveString(para->ID, recvBuffer);
		fstream output;
		pthread_mutex_lock(&mtx);
		output.open("log.txt", ios::app | ios::out);
		output << recvBuffer << endl;
		output.close();
		pthread_mutex_lock(&mtx);
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

		ServerLog temp;
		temp.SetLog(string(name), time, content);

		//save to server log
		fstream output;
		output << temp;
*/
		memset(recvBuffer, 0, sizeof(recvBuffer));
	}
}

void* Admin::Output(void* args)
{
	cout << "Call Output Thread" << endl;
	Parameter *para = (Parameter*)args;
	char sendBuffer[200];
	memset(sendBuffer, 0, sizeof(sendBuffer));
	
	ifstream input;
	streampos p=0;
	while (para->pThis->ClientList[para->ID].Online)
	{
		//read from log
		pthread_mutex_lock(&mtx);
		input.open("log.txt", ios::in);
		input.seekg(p);
		input.getline(sendBuffer, 200);
		if (strlen(sendBuffer) == 0)
		{
			input.close();
			pthread_mutex_unlock(&mtx);
			continue;
		}
		p = input.tellg();
		input.close();
		pthread_mutex_unlock(&mtx);

		//send
		para->pThis->SendString(para->ID, sendBuffer);
		memset(sendBuffer, 0, sizeof(sendBuffer));
	}
}

void* Admin::ReceiveRequest(void* args)
{
	Admin* para = (Admin *)args;
	while (para->Online) //server is online
	{
		//accept
		Client tmp;
		socklen_t addrClientSize = sizeof(sockaddr_in);
		tmp.sockClient = accept(para->sockServer, (sockaddr*)&(tmp.addrClient), &addrClientSize);
		tmp.Online = true;
		para->ClientList.push_back(tmp);
		
		//client is online
		char ipBuff[INET_ADDRSTRLEN];
		printf("User IP:%s connected..\n", inet_ntop(AF_INET, &(tmp.addrClient.sin_addr),ipBuff,sizeof(ipBuff)));

		//thread to receive client messages and send server logs
		Parameter putPara;
		pthread_t tmpThread;
		putPara.pThis = para;
		putPara.ID = Client::ClientNum;

		pthread_create(&tmpThread, NULL, Input, (void *)&para);
		ReceiveThread.push_back(tmpThread);
		pthread_create(&tmpThread, NULL, Output, (void *)&para);
		SendThread.push_back(tmpThread);

		Client::ClientNum++;
	}
}

File::File()
{
	Path = (char*)"log.txt";
}

void File::SetPath(char* path)
{
	Path = path;
}

fstream& operator<<(fstream& output, ServerLog& log)
{
	//file lock
    pthread_mutex_lock(&mtx);
	cout << "Output locks file" << endl;

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
	pthread_mutex_unlock(&mtx);

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

	pthread_mutex_lock(&mtx);
	//write new logs to the log file
	output.open(log.Path, ios::out|ios::app);
	for (i = 0; i < alltemp.size(); ++i)
	{
		output << alltemp[i].Name << " "
			<< alltemp[i].rawTime << " "
			<< alltemp[i].Content << endl;
	}
	output.close();
	
	pthread_mutex_unlock(&mtx);
	cout << "Output unlocks file" << endl;
	return output;
}

fstream& operator>>(fstream& input, ServerLog& log)
{
	pthread_mutex_lock(&mtx);
	cout << "Input locks file" << endl;

	input.open(log.Path, ios::in);
	input >> log.LogContent.Name >> log.LogContent.rawTime >> log.LogContent.Content;

	input.close();
	
	pthread_mutex_unlock(&mtx);
	cout << "Output unlocks file" << endl;
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