#pragma once
//#include "stdafx.h"
#include <WinSock2.h>
#ifndef MYBIND
#define MYBIND

class MyBind {
public:
	static int Bind(SOCKET server_socket_fd, struct sockaddr_in server_addr) {
		return bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	}
};

#endif // !MYBIND
