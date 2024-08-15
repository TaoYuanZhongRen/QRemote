#pragma once
#include<winsock2.h>
#include<iostream>
#include<Windows.h>

using namespace std;
#pragma comment(lib,"WS2_32.lib")


class IOCPServer
{
public:
	IOCPServer();
	~IOCPServer();

	SOCKET m_ListenSocket;
	HANDLE m_ListenThreadHandle;
};

