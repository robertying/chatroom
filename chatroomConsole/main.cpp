#include "chatroom.h"
using namespace std;
int main()
{
//	User user;
//	user.InitializeClient();
//	user.CreateClientSocket();
//	user.ConnectClientSocket();
	cout << "Connected" << endl;
	
	cout << "Name:";//
	char name[100];//TODO

//	user.SetName(string(name));

//	user.SendString(name);
	
	void threadfuc1();
	void threadfuc2();

	thread Thread1(threadfuc1);
	thread Thread2(threadfuc2);
	Thread1.join();
	Thread2.join();

//	thread InputThread(mem_fn(&User::Input), user);
//	thread OutputThread(mem_fn(&User::Output), user);
//	InputThread.join();
//	OutputThread.join();

//	user.CloseClientSocket();

	return 0;
}
void threadfuc1()
{
	time_t rawtime;
	struct tm timeinfo;
	char* str[5];

	int i;
	for (i = 0; i < 5; ++i)
	{
		time(&rawtime);
		timeinfo = *localtime(&rawtime);
		str[i] = asctime(&timeinfo);
	}

	ClientLog log1;
	log1.SetPath("test.txt");
	log1.SetLog("1", 1111111111, "hellowold");
	ClientLog log2;
	log2.SetPath("test.txt");
	log2.SetLog("2", 22222, "hellowold");
	ClientLog log3;
	log3.SetPath("test.txt");
	log3.SetLog("3", 4444, "hellowold");
	ClientLog log4;
	log4.SetPath("test.txt");
	log4.SetLog("4", 33, "hellowold");
	ClientLog log5;
	log5.SetPath("test.txt");
	log5.SetLog("5", 22, "hellowold");

	fstream Output;
	Output << log1;
	Output << log2;
	Output << log3;
	Output << log4;
    Output << log5;


}
void threadfuc2()
{
	ClientLog log1;
	log1.SetPath("test.txt");
	log1.SetLog("6", 1, "hellowold");
	ClientLog log2;
	log2.SetPath("test.txt");
	log2.SetLog("7", 5, "hellowold");
	ClientLog log3;
	log3.SetPath("test.txt");
	log3.SetLog("8", 7, "hellowold");
	ClientLog log4;
	log4.SetPath("test.txt");
	log4.SetLog("9", 5, "hellowold");
	ClientLog log5;
	log5.SetPath("test.txt");
	log5.SetLog("10", 9, "hellowold");

	fstream Output;

	Output << log1;
	Output << log2;
	Output << log3;
	Output << log4;
	Output << log5;
}