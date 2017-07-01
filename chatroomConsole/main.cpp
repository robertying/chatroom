#include "chatroom.h"

int main()
{
	//acquire user name
	User user;
	cout << "Name: ";// TO BE REMOVED
	char name[20];
	cin.getline(name,20); //FROM GUI .. NO SPACE
	while (!user.SetName(name))
	{
		cout << "Name again: ";
		cin.getline(name, 20);
	}

	//recreate log
	fstream output;
	output.open(string(name)+"_log.txt", ios::out);
	output.close();

	//initialize client socket
	user.InitializeClient();
	user.CreateClientSocket();
	user.ConnectClientSocket(); //TO DO : IS SERVER ONLINE? CONNECTION LOST
	
	//threads to input and output logs
	thread OutputThread(&User::Output, user);
	thread InputThread(&User::Input, user);
	InputThread.join();
	OutputThread.join();

	//close client socket
	user.CloseClientSocket();

	return 0;
}