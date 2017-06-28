#include "chatroom.h"

int main()
{
	//initialize client socket
	User user;
	user.InitializeClient();
	user.CreateClientSocket();
	user.ConnectClientSocket(); //TO DO : IS SERVER ONLINE? CONNECTION LOST
	cout << "Connected" << endl; //TO BE REMOVED
	
	//acquire user name
	cout << "Name:";// TO BE REMOVED
	char name[20];
	cin >> name;
	user.SetName(name);

	//threads to input and output logs
	thread InputThread(mem_fn(&User::Input), user);
	thread OutputThread(mem_fn(&User::Output), user);
	InputThread.join();
	OutputThread.join();

	//close client socket
	user.CloseClientSocket();

	return 0;
}