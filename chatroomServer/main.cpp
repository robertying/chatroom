#include "chatroom.h"

int main()
{
	//initialize server
	Admin admin;
	admin.CreateServerSocket();
	admin.ConnectServerSocket();

	//thread to accept clients
	pthread_t threadRequest;
	pthread_create(&threadRequest, NULL, admin.ReceiveRequest, (void*)&admin);

	//thread to receive client messages and send server logs
	int i;
	int ThreadNum = 0;
	vector <pthread_t> ReceiveThread;
	vector <pthread_t> SendThread;
	for (i = ThreadNum; i < Client::ClientNum; ++i)
	{
		Parameter para;
		para.pThis = &admin;
		para.ID = ThreadNum;
		pthread_t tmp;
		pthread_create(&tmp, NULL, admin.Input, (void *)&para);
		ReceiveThread.push_back(tmp);
		pthread_create(&tmp, NULL, admin.Output, (void *)&para);
		SendThread.push_back(tmp);
		ThreadNum++;
	}

	//wait for threads to end
	void* status;
	pthread_join(threadRequest, &status);
	for (i = 0; i < Client::ClientNum; ++i)
	{
		pthread_join(ReceiveThread[i], &status);
		pthread_join(SendThread[i], &status);
	}

	//close server socket and offline
	admin.CloseServerSocket();
	admin.Online = false;

	return 0;
}