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
