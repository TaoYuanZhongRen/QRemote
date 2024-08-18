#include "IOCPServer.h"
#include "_CriticalSection.h"

IOCPServer::IOCPServer()
	:m_ListenSocket(INVALID_SOCKET), m_ListenThreadHandle(NULL),
	m_ListenEventHandle(WSA_INVALID_EVENT), m_CompletionPortHandle(INVALID_HANDLE_VALUE),
	m_KillEventHandle(NULL), m_CriticalSection(), m_FreeContextObjectList(NULL)
{
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		return;
	}

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

	//初始化IOCP(当异步请求完成的时候)
	//创建完成端口
	//启动工作线程(守候完成端口上等待异步请求的完成)


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
				This->OnAccept();
			}
			else
				break;
		}
		else
			break;
	}
	
	return 0;
}

void IOCPServer::OnAccept()
{
	int Result;

	//保存上线用户IP地址
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN ClientAddress = { 0 };
	int ClientAddressLength = sizeof(SOCKADDR_IN);
	ClientSocket = accept(m_ListenSocket,
		(sockaddr*)&ClientAddress,
		&ClientAddressLength);

	if (ClientSocket == SOCKET_ERROR)
	{
		return;
	}
	//为每一个到达的信号维护了一个与之关联的数据结构，简称为用户的上下背景文
	PCONTEXT_OBJECT ContextObject = AllocateContextObject();
	if (ContextObject == NULL)
	{
		closesocket(ClientSocket);
		ClientSocket = INVALID_SOCKET;
		return;
	}
	//成员赋值
	ContextObject->ClientSocket = ClientSocket;
	//关联内存
	ContextObject->RecvWsaBuffer.buf = static_cast<char*>(ContextObject->BufferData);
	ContextObject->RecvWsaBuffer.len = sizeof(ContextObject->BufferData);

	HANDLE Handle = CreateIoCompletionPort((HANDLE)ClientSocket, m_CompletionPortHandle, (ULONG_PTR)ContextObject, 0);
	if (Handle != m_CompletionPortHandle)
	{
		delete ContextObject;
		ContextObject = nullptr;
		if (ClientSocket != INVALID_SOCKET)
		{
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
		}
		return;
	}
	

	// 保持连接检测,对方主机是否崩溃.
	// 如果2小时内在此套接口的任一方向都没有数据交换,TCP就自动给对方发一个数据包保持存活
	// 在做服务器时，如果发生客户端网线或断电等非正常断开的现象，如果服务器没有设置S0KEERALIVE选项,
	// 则会一直不关闭SOCKET。因为上的的设置是默认两个小时时间太长了所以我们就修正这个值
	// 设置套接字的选项卡 SetKeepAlive 开启保活机制 SOKEEPALIVE
	m_KeepAliveTime = 3;
	bool IsOk = true;
	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsOk, sizeof(IsOk)))
	{

	}
	//设置超市详细信息
	tcp_keepalive keepAlive;
	keepAlive.onoff = 1;                               //启用保活
	keepAlive.keepalivetime = m_KeepAliveTime;         //超过三分钟没有数据就发送探测包
	keepAlive.keepaliveinterval = 1000 * 10;        //重试间隔为10s

	WSAIoctl(ContextObject->ClientSocket,
		SIO_KEEPALIVE_VALS,
		&keepAlive,
		sizeof(keepAlive),
		NULL,
		0,
		(unsigned long*)&IsOk,
		0,
		NULL);

	_CriticalSection CriticalSection(&m_CriticalSection);

	m_ConnectionContextObjectList.push_back(ContextObject);   //插入到内存列表中
	OVERLAPPEDEX* OverLappedEx = new OVERLAPPEDEX(IO_INITIALIZE);
	IsOk = false;
	//向完成端口投递一个请求
	//工作线程会等待完成端口的完成状态
	IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (ULONG_PTR)ContextObject, &OverLappedEx->m_OverLapped);
	if (!IsOk && GetLastError() != ERROR_IO_PENDING)
	{
		//如果投递失败
		RemoveContextObject(ContextObject);
		return;
	}
	//该上线用户完成了上线的请求
	//服务器向该用户投递PostRecv请求
	PostRecv(ContextObject);


}
void IOCPServer::PostRecv(PCONTEXT_OBJECT ContextObject)
{
	//向我们的刚上线的用户的投递一个接受数据的请求
	//如果该请求得到完成(用户发送数据)
	//工作线程(守候在完成端口)会响应并调用HandleIO函数
	OVERLAPPEDEX* OverlappedEx = new OVERLAPPEDEX(IO_RECEIVE);
	DWORD ReturnLength;
	ULONG Flags = MSG_PARTIAL;
	int IsOk = WSARecv(ContextObject->ClientSocket,
		&ContextObject->RecvWsaBuffer,					//接受数据的内存
		1,
		&ReturnLength,									// TransferBufferLength 
		&Flags,
		&OverlappedEx->m_OverLapped, 
		NULL);
	if (IsOk == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		RemoveContextObject(ContextObject);
	}
}

void IOCPServer::RemoveContextObject(PCONTEXT_OBJECT ContextObject)
{
	_CriticalSection CriticalSection(&m_CriticalSection);
	
	//在内存中查找该用户的上下背景文数据结构
	auto i = std::find(m_ConnectionContextObjectList.begin(), m_ConnectionContextObjectList.end(), ContextObject);
	if (i != m_ConnectionContextObjectList.end())
	{
		//取消在当前套接字的异步I0 以前的未完成的异步请求全部立即取消
		CancelIo((HANDLE)ContextObject->ClientSocket);
		//关闭套接字
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;

		//判断还有没有异步I0请求在当前套接字上
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObject)) 
		{
			Sleep(0);
		}
		//将该内存结构回收至内存池
		MoveContextObjectToFreeList(ContextObject); 
	}
	
}
void IOCPServer::MoveContextObjectToFreeList(PCONTEXT_OBJECT ContextObject)
{
	_CriticalSection CriticalSection(&m_CriticalSection);

	auto i = std::find(m_ConnectionContextObjectList.begin(), m_ConnectionContextObjectList.end(), ContextObject);
	if (i != m_ConnectionContextObjectList.end())
	{
		memset(ContextObject->BufferData, 0, PACKET_LENGTH);
		m_FreeContextObjectList.push_back(ContextObject);
		m_ConnectionContextObjectList.remove(ContextObject);
	}
}
PCONTEXT_OBJECT IOCPServer::AllocateContextObject()
{
	PCONTEXT_OBJECT ContextObject = NULL;

	//进入临界区
	_CriticalSection CriticalSection(&m_CriticalSection);
	//判断内存池是否为空
	if (m_FreeContextObjectList.empty() == false)
	{
		ContextObject = m_FreeContextObjectList.front();
		m_FreeContextObjectList.pop_front();
	}
	else
	{
		ContextObject = new CONTEXT_OBJECT();
	}
	if (ContextObject != NULL)
	{
		ContextObject->InitMember();
	}

	return ContextObject;
}
