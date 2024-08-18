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
	//�ȴ������߳��˳�
	WaitForSingleObject(m_ListenThreadHandle,INFINITE);

	//���ո�����Դ
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
	//�����˳������̵߳��¼�
	m_KillEventHandle = CreateEvent(NULL, false, false, NULL);
	if (m_KillEventHandle == NULL)
	{
		return false;
	}
	//����һ�������׽���
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	bool Result;
	if (m_ListenSocket == INVALID_SOCKET)
	{
		return FALSE;
	}
	//����һ�������¼�
	m_ListenEventHandle = WSACreateEvent();
	//creates a manual-reset event object with an initial state of nonsignaled
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		goto Error;
	}
			
	//�¼�ѡ��ģ��
	//�������׽������¼����й���,������FD_ACCEPT  FD_CLOSE������
	Result = WSAEventSelect(m_ListenSocket,	m_ListenEventHandle, FD_ACCEPT | FD_CLOSE);

	if (Result == SOCKET_ERROR)
	{
		goto Error;
	}
	//��ʼ��Server������
	SOCKADDR_IN ServerAddress;
	ServerAddress.sin_port = htons(ListenPort);
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	Result = bind(m_ListenSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress));
	if (Result == SOCKET_ERROR)
	{
		goto Error;
	}

	//��ʼ����
	Result = listen(m_ListenSocket, SOMAXCONN);
	if (Result == SOCKET_ERROR)
	{
		goto Error;
	}

	//���������߳�
	m_ListenThreadHandle = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenThreadProcedure, (void*)this, 0, NULL); //���̴߳���������thisָ��
	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)        
		goto Error;

	//��ʼ��IOCP(���첽������ɵ�ʱ��)
	//������ɶ˿�
	//���������߳�(�غ���ɶ˿��ϵȴ��첽��������)


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
			//��������������m_KillEventHandle�¼�
			break;
		}
		DWORD Result;

		//�ȴ������¼����ţ��׽������ţ�
		Result = WSAWaitForMultipleEvents(1, &This->m_ListenEventHandle, false, 100, false);
		if(Result == WSA_WAIT_TIMEOUT)
			continue;

		//������FD_ACCEPT����FD_CLOSE�¼�
		//����¼����ţ���ת��һ�������¼�������
		Result = WSAEnumNetworkEvents(This->m_ListenSocket,
			This->m_ListenEventHandle, &NetWorkEvents);
		if (Result == SOCKET_ERROR)
			break;

		if (NetWorkEvents.lNetworkEvents & FD_ACCEPT)
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				//����ͻ�����������
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

	//���������û�IP��ַ
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
	//Ϊÿһ��������ź�ά����һ����֮���������ݽṹ�����Ϊ�û������±�����
	PCONTEXT_OBJECT ContextObject = AllocateContextObject();
	if (ContextObject == NULL)
	{
		closesocket(ClientSocket);
		ClientSocket = INVALID_SOCKET;
		return;
	}
	//��Ա��ֵ
	ContextObject->ClientSocket = ClientSocket;
	//�����ڴ�
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
	

	// �������Ӽ��,�Է������Ƿ����.
	// ���2Сʱ���ڴ��׽ӿڵ���һ����û�����ݽ���,TCP���Զ����Է���һ�����ݰ����ִ��
	// ����������ʱ����������ͻ������߻�ϵ�ȷ������Ͽ����������������û������S0KEERALIVEѡ��,
	// ���һֱ���ر�SOCKET����Ϊ�ϵĵ�������Ĭ������Сʱʱ��̫�����������Ǿ��������ֵ
	// �����׽��ֵ�ѡ� SetKeepAlive ����������� SOKEEPALIVE
	m_KeepAliveTime = 3;
	bool IsOk = true;
	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsOk, sizeof(IsOk)))
	{

	}
	//���ó�����ϸ��Ϣ
	tcp_keepalive keepAlive;
	keepAlive.onoff = 1;                               //���ñ���
	keepAlive.keepalivetime = m_KeepAliveTime;         //����������û�����ݾͷ���̽���
	keepAlive.keepaliveinterval = 1000 * 10;        //���Լ��Ϊ10s

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

	m_ConnectionContextObjectList.push_back(ContextObject);   //���뵽�ڴ��б���
	OVERLAPPEDEX* OverLappedEx = new OVERLAPPEDEX(IO_INITIALIZE);
	IsOk = false;
	//����ɶ˿�Ͷ��һ������
	//�����̻߳�ȴ���ɶ˿ڵ����״̬
	IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (ULONG_PTR)ContextObject, &OverLappedEx->m_OverLapped);
	if (!IsOk && GetLastError() != ERROR_IO_PENDING)
	{
		//���Ͷ��ʧ��
		RemoveContextObject(ContextObject);
		return;
	}
	//�������û���������ߵ�����
	//����������û�Ͷ��PostRecv����
	PostRecv(ContextObject);


}
void IOCPServer::PostRecv(PCONTEXT_OBJECT ContextObject)
{
	//�����ǵĸ����ߵ��û���Ͷ��һ���������ݵ�����
	//���������õ����(�û���������)
	//�����߳�(�غ�����ɶ˿�)����Ӧ������HandleIO����
	OVERLAPPEDEX* OverlappedEx = new OVERLAPPEDEX(IO_RECEIVE);
	DWORD ReturnLength;
	ULONG Flags = MSG_PARTIAL;
	int IsOk = WSARecv(ContextObject->ClientSocket,
		&ContextObject->RecvWsaBuffer,					//�������ݵ��ڴ�
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
	
	//���ڴ��в��Ҹ��û������±��������ݽṹ
	auto i = std::find(m_ConnectionContextObjectList.begin(), m_ConnectionContextObjectList.end(), ContextObject);
	if (i != m_ConnectionContextObjectList.end())
	{
		//ȡ���ڵ�ǰ�׽��ֵ��첽I0 ��ǰ��δ��ɵ��첽����ȫ������ȡ��
		CancelIo((HANDLE)ContextObject->ClientSocket);
		//�ر��׽���
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;

		//�жϻ���û���첽I0�����ڵ�ǰ�׽�����
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObject)) 
		{
			Sleep(0);
		}
		//�����ڴ�ṹ�������ڴ��
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

	//�����ٽ���
	_CriticalSection CriticalSection(&m_CriticalSection);
	//�ж��ڴ���Ƿ�Ϊ��
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
