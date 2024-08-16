#include "IOCPServer.h"

IOCPServer::IOCPServer()
{
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		return;
	}
	m_ListenSocket = INVALID_SOCKET;
	m_ListenThreadHandle = NULL;
	m_ListenEventHandle = WSA_INVALID_EVENT;
	m_CompletionPortHandle = INVALID_HANDLE_VALUE;
	m_KillEventHandle = NULL;

	InitializeCriticalSection(&m_CriticalSection);
}

IOCPServer::~IOCPServer()
{
	SetEvent(m_KillEventHandle);
	//等待监听线程退出
	WaitForSingleObject(m_ListenThreadHandle,INFINITE);

	//回收各种资源
	if (m_ListenSocket != INVALID_SOCKET)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}
	if (m_ListenEventHandle != WSA_INVALID_EVENT)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
	}
	if (m_CompletionPortHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_CompletionPortHandle);
		m_CompletionPortHandle = INVALID_HANDLE_VALUE;
	}

	DeleteCriticalSection(&m_CriticalSection);
	WSACleanup();

}

bool IOCPServer::ServerRun(unsigned short ListenPort)
{
	//创建退出监听线程的事件
	m_KillEventHandle = CreateEvent(NULL, false, false, NULL);
	if (m_KillEventHandle == NULL)
	{
		return false;
	}
	//创建一个监听套接字
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	bool Result;
	if (m_ListenSocket == INVALID_SOCKET)
	{
		return FALSE;
	}
	//创建一个监听事件
	m_ListenEventHandle = WSACreateEvent();
	//creates a manual-reset event object with an initial state of nonsignaled
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		goto Error;
	}
			
	//事件选择模型
	//将监听套接字与事件进行关联,并授予FD_ACCEPT  FD_CLOSE的属性
	Result = WSAEventSelect(m_ListenSocket,	m_ListenEventHandle, FD_ACCEPT | FD_CLOSE);

	if (Result == SOCKET_ERROR)
	{
		goto Error;
	}
	//初始化Server端网卡
	SOCKADDR_IN ServerAddress;
	ServerAddress.sin_port = htons(ListenPort);
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	Result = bind(m_ListenSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress));
	if (Result == SOCKET_ERROR)
	{
		goto Error;
	}

	//开始监听
	Result = listen(m_ListenSocket, SOMAXCONN);
	if (Result == SOCKET_ERROR)
	{
		goto Error;
	}

	//创建监听线程
	m_ListenThreadHandle = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenThreadProcedure, (void*)this, 0, NULL); //向线程处理函数传递this指针
	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)        
		goto Error;




Error:
	WSACloseEvent(m_ListenEventHandle);
	m_ListenEventHandle = WSA_INVALID_EVENT;
	closesocket(m_ListenSocket);
	m_ListenSocket = INVALID_SOCKET;
	return false;
}

DWORD __stdcall IOCPServer::ListenThreadProcedure(LPVOID ParameterData)
{
	IOCPServer* This = static_cast<IOCPServer*>(ParameterData);

	int EventIndex = 0;
	WSANETWORKEVENTS NetWorkEvents;
	while (true)
	{
		EventIndex = WaitForSingleObject(This->m_KillEventHandle, 100);
		if (EventIndex == WAIT_OBJECT_0)
		{
			//由析构函数触发m_KillEventHandle事件
			break;
		}
		DWORD Result;

		//等待监听事件授信（套接字授信）
		Result = WSAWaitForMultipleEvents(1, &This->m_ListenEventHandle, false, 100, false);
		if(Result == WSA_WAIT_TIMEOUT)
			continue;

		//发生了FD_ACCEPT或者FD_CLOSE事件
		//如果事件授信，就转成一个网络事件来处理
		Result = WSAEnumNetworkEvents(This->m_ListenSocket,
			This->m_ListenEventHandle, &NetWorkEvents);
		if (Result == SOCKET_ERROR)
			break;

		if (NetWorkEvents.lNetworkEvents & FD_ACCEPT)
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				//处理客户端上线请求
				//This->OnAccept();
			}
			else
				break;
		}
		else
			break;


	}

	return 0;
}
