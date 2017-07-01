#include "chatroom.h"
extern vector <pthread_t> ReceiveThread;
extern vector <pthread_t> SendThread;

int main()
{
	//initialize server
	Admin admin;
	admin.CreateServerSocket();
	admin.ConnectServerSocket();

	//create log.txt
	fstream log("log.txt", ios::app | ios::out);
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