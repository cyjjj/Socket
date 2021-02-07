#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <pthread.h>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include "protocol_analyzer.h"

#pragma comment(lib, "ws2_32.lib")

#define MAX_LENGTH 1024
using namespace std;

void Connect();
void Close();
void Request(int); //request information from server
void Send();	   //sending message to other clients
void Exit();
void alterLock(bool); //change the state whether the request is satisfied

SOCKET sClient;
pthread_mutex_t mutex;
bool connectedToServer = false;
bool waitingStatus = false;
const int waitTime = 200; //200ms=0.2s
const int maxWaitNum = 100; //max wait time=20s
pthread_t receiver;
bool isexit = false;

int time_reply_count = 0;

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;
	int op; // operation chosen

	// initializing WinSock
	wVersionRequested = MAKEWORD(2, 2); // expected WinSock DLL version
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0)
	{
		printf("WSAStartup() failed!\n");
		return 0;
	}
	// confirm version 2.2 is supported
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid Winsock version!\n");
		return 0;
	}

	// create socket, encapsulate TCP
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return 0;
	}

	pthread_mutex_init(&mutex, NULL);
	//get the input from user
	while (true)
	{
		//menu
		cout << "Operation list:" << endl;
		cout << "1)\tConnect" << endl;
		if (connectedToServer)
		{
			cout << "2)\tClose" << endl;
			cout << "3)\tRequest time" << endl;
			cout << "4)\tRequest name" << endl;
			cout << "5)\tRequest client list" << endl;
			cout << "6)\tSend message" << endl;
		}
		cout << "7)\tExit" << endl;
		cout << ">>";
		scanf("%d",&op);
		fflush(stdin);
		if (op < 1 || op > 7 || (!connectedToServer && 2 <= op && op <= 6))
		{
			printf("ERROR:\tInvalid operation!\n");
			continue;
		}
		if (op == 1)
			Connect();
		else if (op == 2)
			Close();
		else if (op >= 3 && op <= 5)
			Request(op); //request time/name/list
		else if (op == 6)
			Send();
		else if (op == 7){
			if (connectedToServer){
				Close();							
			}
			break;
		}
	}
	return 0;
}

void alterLock(bool type)
{
	pthread_mutex_lock(&mutex);
	waitingStatus = type;
	pthread_mutex_unlock(&mutex);
}

void *receive(void *args)
{
	SOCKET *sClient = (SOCKET *)args;
	
	while (true)
	{
		char buffer_recv[MAX_LENGTH] = {0};
		int ret = recv(*sClient,buffer_recv,MAX_LENGTH,0);
		if (ret == SOCKET_ERROR || ret == 0)
			break;
			
		int t = (int)time(NULL);
		time_t curtime = time(NULL);
    	tm *ptm = localtime(&curtime);
    	char buf[64] = {0};
    	sprintf(buf, "%d/%02d/%02d %02d:%02d:%02d", ptm->tm_year+1900, ptm->tm_mon+1,ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
		
        string pstr = buffer_recv;
        if(strcmp(pstr.c_str(),"\n")!=0) {
        	Mypacket pack = analyzePacket(pstr);
        	
        	if(pack.getType() >= 4 && pack.getType() <= 7)
				cout << "[" << buf << "]" << "Recieve a packet." << endl;
		
			//not a reply pack
			if(pack.getType() < 0 || pack.getType() > 7) {
				cout << pstr << endl;
			}
			
			switch (pack.getType()) {
        		case 0b100: { //time
            		printf("Current server time: %s\n", pack.getContent().c_str());
            		time_reply_count++;
            		printf("time reply count:%d\n",time_reply_count);
					alterLock(false);
           	 	break;
        		}
        		case 0b101: { //name
            		printf("Client computer name: %s\n", pack.getContent().c_str());
					alterLock(false);
            		break;
        		}
        		case 0b110: { //client list
            		puts("Client list:");
					printf("%s\n", pack.getContent().c_str());
					alterLock(false);
            		break;
       			}
        		case 0b111: { //send Message
        			puts("Message:");
            		printf("%s\n", pack.getContent().c_str());
					alterLock(false);
            		break;
        		}
    		}
   	 	}
	}
}

void Connect()
{
	if (connectedToServer)
	{
		printf("Already connected.\n");
		return;
	}

	static char ip[MAX_LENGTH];
	int port;
	cout << "Input IP (default: 127.0.0.1): ";
	cin >> ip;
	fflush(stdin);

	cout << "Input port (default: 3380): ";
	cin >> port;
	fflush(stdin);

	struct sockaddr_in saServer;
	//information
	saServer.sin_family = AF_INET;
	saServer.sin_port = htons(port); //order of number
	saServer.sin_addr.S_un.S_addr = inet_addr(ip);

	int ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		puts("ERROR:\tConnection failed!");
		closesocket(sClient);
		WSACleanup();
		return;
	}
	else
		puts("SUCCESS:\tConnection Successful!");

	connectedToServer = true;

	ret = pthread_create(&receiver, NULL, receive, &sClient);
	if (ret != 0)
	{
		printf("ERROR:\tCreate pthread failed; Return code: %d\n", ret);
		return;
	}
	puts("");
	
	//send a package for connecting successfully
	Mypacket pack = Mypacket(1,""); //connect
	string pstr = pack.toPacket(); //
	send(sClient, pstr.c_str(), pstr.size(), 0);
}

void Close()
{
	//send a package for closing the connection
	Mypacket pack = Mypacket(0,""); //close
	string pstr = pack.toPacket(); //
	send(sClient, pstr.c_str(), pstr.size(), 0);
	
	closesocket(sClient);
	connectedToServer = false;
	pthread_join(receiver, NULL);
	if (!isexit)
	{
		sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sClient == INVALID_SOCKET)
		{
			WSACleanup();
			printf("ERROR: Require Socket failed.\n");
			exit(0);
		}
	}
	printf("Close successfully\n");
}

void Request(int op)
{
	int type = op + 1;
	//if(op == 3) type = 4;//time:100
	//else if(op == 4) type = 5;//name:101
	//else if(op == 5) type = 6; //client list:110
	
	//send a package for requesting infomation
	//for(int i=0;i<100;i++) { //100 times
		Mypacket pack = Mypacket(type,""); //request
		string pstr = pack.toPacket(); //
		send(sClient, pstr.c_str(), pstr.size(), 0);

		alterLock(true);	
		for(;;){ //wait for reply until receive a package
			pthread_mutex_lock(&mutex);
			if (!waitingStatus){
				pthread_mutex_unlock(&mutex);
				break;
			}
			pthread_mutex_unlock(&mutex);
			Sleep(waitTime);
		}
	//}
}

void Send()
{
	//read in the client sent messages to
	int id;
	puts("Input client id sent to:");
	scanf("%d", &id);
	fflush(stdin);
	//read in the message
	puts("Input message (end with '#'):");
	char buf[MAX_LENGTH] = {0};
	scanf("%[^#]",buf); //end with '#'
	fflush(stdin);
	string content = buf;
	 
	//send a package for sending messages
	Mypacket pack = Mypacket(7, content, id); //send messages
	string pstr = pack.toPacket(); //
	send(sClient, pstr.c_str(), pstr.size(), 0);
	//cout << "pstr.c_str()=" << pstr.c_str() << endl;
	
	alterLock(true);	
	for(;;){ //wait for reply until receive a package
		pthread_mutex_lock(&mutex);
		if (!waitingStatus){
			pthread_mutex_unlock(&mutex);
			break;
		}
		pthread_mutex_unlock(&mutex);
		Sleep(waitTime);
	}
}

void Exit()
{
	if (connectedToServer)
		Close();
	exit(0);
}
