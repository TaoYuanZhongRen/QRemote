#pragma once
#include<winsock2.h>
#include<iostream>
//#include<Windows.h>

using namespace std;
#pragma comment(lib,"WS2_32.lib")


class IOCPServer
{
public:
	IOCPServer();
	~IOCPServer();


public:
	bool ServerRun(unsigned short ListenPort);

	static DWORD WINAPI ListenThreadProcedure(LPVOID ParameterData /*LPVOID 这里的L指的是相对于16位，所以这个类型是指32位的泛型指针*/);
private:
	SOCKET m_ListenSocket;
	HANDLE m_ListenThreadHandle;
	HANDLE m_ListenEventHandle;         //监听事件
	HANDLE m_KillEventHandle;               //监听线程退出
	CRITICAL_SECTION m_CriticalSection;  //临界区
	HANDLE m_CompletionPortHandle;

};

