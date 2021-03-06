#include "chatroom.h"
extern vector <pthread_t> ReceiveThread;
extern vector <pthread_t> SendThread;
extern char* logPath;

int main()
{
	//initialize server
	Admin admin;
	admin.CreateServerSocket();
	admin.ConnectServerSocket();

	//create log.txt
	time_t tTime = time(NULL);
	char* dt = ctime(&tTime);
	logPath = strcat(dt,"_log.txt");

	fstream log(logPath, ios::app | ios::out);
	log.close();

	//thread to accept clients
	pthread_t threadRequest;
	pthread_create(&threadRequest, NULL, admin.ReceiveRequest, (void*)&admin);

	//wait for threads to end
	int i;
	void* status;
	pthread_join(threadRequest, &status);
	for (i = 0; i < Client::ClientNum; ++i)
	{
		pthread_join(ReceiveThread[i], &status);
		pthread_join(SendThread[i], &status);
	}

	//close server socket and offline
	admin.CloseServerSocket();

	return 0;
}