#include "chatroom.h"
mutex mtx;
bool Server::Online = false;

int Client::InitializeClient()
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
	cout << "InitializeClient" << endl;
	return 0;
}

void Client::CreateClientSocket()
{
	//create socket
	sockClient = socket(AF_INET, SOCK_STREAM, 0);
	
	//set server address
	addrServ.sin_addr.S_un.S_addr = inet_addr("45.55.3.170");
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(42001);
	cout << "CreateClientSocket" << endl;
}

void Client::ConnectClientSocket()
{
	//connect
	connect(sockClient, (SOCKADDR *)&addrServ, sizeof(SOCKADDR));
	Online = true;
	cout << "ConnectClientSocket" << endl;
}

void Client::CloseClientSocket()
{
	//close and clean
	closesocket(sockClient);
	Online = false;
	WSACleanup();
	cout << "CloseClientSocket" << endl;
}

User::User()
{
	Name = "Unnamed User";
	Online = false;
}

bool User::SetName(string name)
{
	//non-empty check
	if (name.find(' ') != string::npos) return false;
	//length check
	if (name.length() > 19) return false;

	Name = name;
	return true;
}

void User::SendString(char* StringToSend)
{
	int status;
	status=send(sockClient, StringToSend, strlen(StringToSend), 0);
	if (status <= 0)
	{
		Server::Online = false;
		CloseClientSocket();
		ConnectionLost();
	}
	cout << "SendString" << endl;
}

void User::ReceiveString(char * StringToReceive)
{
	recv(sockClient, StringToReceive, 100, 0);
	cout << "ReceiveString" << endl;
}

void User::Input()
{
	cout << "Call Input Thread" << endl;
	char inputBuffer[200];
	memset(inputBuffer, 0, sizeof(inputBuffer));
	while (Online)
	{
//		cout << "message:"; //TO BE REMOVED
//		cin.getline(inputBuffer,200,'\n'); //TODO receive text from gui
		strcpy_s(inputBuffer,"hello world!");
		SendString(inputBuffer);
		memset(inputBuffer, 0, sizeof(inputBuffer));
	}
}

void User::Output()
{
	cout << "Call Output Thread" << endl;
	char recvBuffer[200];
	memset(recvBuffer, 0, sizeof(recvBuffer));
	while (Online)
	{
		//receive
		ReceiveString(recvBuffer);

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

		ClientLog temp;
		temp.SetLog(string(name), time, content);

		//save to client log
		fstream output;
		output << temp;

		memset(recvBuffer, 0, sizeof(recvBuffer));
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
	Path = "log.txt";
}

void File::SetPath(char* path)
{
	Path = path;
}

fstream& operator<<(fstream& output, ClientLog& log)
{
	cout << "Call <<operator" << endl;
	//file lock
	mtx.lock();

	//read from log file
	output.open(log.Path, ios::in|ios::app);
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
			alltemp.insert(alltemp.begin()+i, log.LogContent);
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

fstream& operator>>(fstream& input, ClientLog& log)
{
	cout << "Call >>operator" << endl;
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