#include "IOCPServer.h"

IOCPServer::IOCPServer()
{
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		return;
	}

}

IOCPServer::~IOCPServer()
{
}
