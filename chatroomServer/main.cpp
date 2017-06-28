#include "chatroom.h"

int main()
{
	//initialize server
	Admin admin;
	admin.CreateServerSocket();
	admin.ConnectServerSocket();

	//thread to accept clients
	pthread_t threadRequest;
	pthread_create(&threadRequest, NULL, admin.ReceiveRequest, 0);

	//thread to receive local logs and save them
	int i;
	int ThreadNum = 0;
	vector <pthread_t> ReceiveThread;
	for (i = ThreadNum; i < Client::ClientNum; ++i)
	{
		pthread_t tmp;
		pthread_create(&tmp, NULL, admin.Input, (void *)&ThreadNum);
		ReceiveThread.push_back(tmp);
		ThreadNum++;
	}

	//close server socket and offline
	admin.CloseServerSocket();
	admin.Online = false;

	//wait for threads to end
	void* status;
	pthread_join(threadRequest,&status);

	return 0;
}