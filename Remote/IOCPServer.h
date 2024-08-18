#pragma once
#include<winsock2.h>
#include<list>
#include<mstcpip.h>

#pragma comment(lib,"WS2_32.lib")

const unsigned long PACKET_LENGTH = 0x2000;

typedef struct CONTEXT_OBJECT
{
	SOCKET ClientSocket;        //当前server使用该套接字与目标客户端通信
	WSABUF RecvWsaBuffer;
	char BufferData[PACKET_LENGTH];

	void InitMember()
	{
		ClientSocket = INVALID_SOCKET;
		memset(&RecvWsaBuffer, 0, sizeof(WSABUF));
		memset(BufferData, 0, sizeof(char)*PACKET_LENGTH);
	}

}CONTEXT_OBJECT, * PCONTEXT_OBJECT;

enum PACKET_TYPE
{
	IO_INITIALIZE,
	IO_RECEIVE,
	IO_SEND,
	IO_IDLE
};

class OVERLAPPEDEX
{
public:
	OVERLAPPED m_OverLapped;
	PACKET_TYPE m_PacketType;

public:

	OVERLAPPEDEX(PACKET_TYPE PackType) :m_OverLapped(), m_PacketType(PackType)
	{
		ZeroMemory(&(this->m_OverLapped), sizeof(OVERLAPPED));
	}
};
typedef std::list<PCONTEXT_OBJECT> CONTEXT_OBJECT_LIST;

class IOCPServer
{
public:
	IOCPServer();
	~IOCPServer();


public:
	bool ServerRun(unsigned short ListenPort);

	static DWORD WINAPI ListenThreadProcedure(LPVOID ParameterData /*LPVOID 这里的L指的是相对于16位，所以这个类型是指32位的泛型指针*/);

	void OnAccept();
	void PostRecv(PCONTEXT_OBJECT ContextObject);
	void RemoveContextObject(PCONTEXT_OBJECT ContextObject);
	
	PCONTEXT_OBJECT AllocateContextObject();
	void MoveContextObjectToFreeList(PCONTEXT_OBJECT ContextObject);

private:
	SOCKET m_ListenSocket;
	HANDLE m_ListenThreadHandle;
	HANDLE m_ListenEventHandle;         //监听事件
	HANDLE m_KillEventHandle;               //监听线程退出
	CRITICAL_SECTION m_CriticalSection;  //临界区
	HANDLE m_CompletionPortHandle;
	CONTEXT_OBJECT_LIST m_FreeContextObjectList;      //内存池链表
	CONTEXT_OBJECT_LIST m_ConnectionContextObjectList;      //上线用户
	unsigned int m_KeepAliveTime;                          //保活机制
};


