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

	static DWORD WINAPI ListenThreadProcedure(LPVOID ParameterData /*LPVOID �����Lָ���������16λ���������������ָ32λ�ķ���ָ��*/);
private:
	SOCKET m_ListenSocket;
	HANDLE m_ListenThreadHandle;
	HANDLE m_ListenEventHandle;         //�����¼�
	HANDLE m_KillEventHandle;               //�����߳��˳�
	CRITICAL_SECTION m_CriticalSection;  //�ٽ���
	HANDLE m_CompletionPortHandle;

};

