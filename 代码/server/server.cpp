// environmentTest.cpp: 定义控制台应用程序的入口点。
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable:4996)
//#include "stdafx.h"
#include "fun.h"
#include "fun1.cpp"
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <thread>
#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll



#define SERVER_PORT 8000
#define BUFFER_SIZE 51200
#define FILE_NAME_MAX_SIZE 512



int main()
{
	/*-----------------------创建服务端监听套接字---------------------*/
	WORD sockVersion = MAKEWORD(2, 2);
	WSAData wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) {
		cout << "WSA初始化失败" << WSAGetLastError() << "\n";
		system("pause");
		return -1;
	}
	cout << "WSA初始化成功\n";
	// 创建UDP套接口 
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	SOCKET server_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	// 绑定套接口 
	if (-1 == MyBind::Bind(server_socket_fd, server_addr))
	{
		perror("Server Bind Failed:");
		system("pause");
		exit(1);
	}

	DWORD TIME_OUT = 1000;
	int err = setsockopt(server_socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&TIME_OUT, sizeof(TIME_OUT));
	if (err != 0) {
		std::cout << WSAGetLastError() << "\n";
	}
	err = setsockopt(server_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&TIME_OUT, sizeof(TIME_OUT));
	if (err != 0) {
		std::cout << WSAGetLastError() << "\n";
	}
	int servePort = 0;
	/*---------------------命令处理----------------------------------------------------- */
	while (1)
	{
		// 定义一个地址，用于捕获客户端地址 

		struct sockaddr_in client_addr;
		int client_addr_length = sizeof(client_addr);
		cout << "WAITING COMMAND...........\n";
		char ins[512];
		memset(ins, '\0', 512);
		while (recvfrom(server_socket_fd, ins, 512, 0, (struct sockaddr*)&client_addr, &client_addr_length) < 0) {};
		cout << ins << endl;
		char addr[20];
		memset(addr, '\0', 20);
		char file_name[20];
		memset(file_name, '\0', 20);
		if (ins[6] == 's')
		{
			int i;
			for (i = 11; ins[i] != ' '; ++i)
			{
				addr[i - 11] = ins[i];
			}
			++i;
			for (int j = 0; i < strlen(ins); ++i, ++j)
			{
				file_name[j] = ins[i];
			}
		}
		else if (ins[6] == 'g')
		{
			int i;
			for (i = 10; ins[i] != ' '; ++i)
			{
				addr[i - 10] = ins[i];
			}
			++i;
			for (int j = 0; i < strlen(ins); ++i, ++j)
			{
				file_name[j] = ins[i];
			}
		}
		cout << "addr: " << addr << endl;
		cout << "file_name: " << file_name << endl;
		char path_server[50];
		memset(path_server, '\0', 50);
		string s1 = "C:\\Users\\admin\\Desktop\\server\\";
		strncpy(path_server, s1.c_str(), s1.size());
		servePort++;
		if (ins[6] == 'g')
		{
			// 发送文件到客户端
			// sent(path_server, file_name, server_socket_fd, client_addr, client_addr_length);
			cout << "servePort: " << servePort << endl;
			thread t1(&sent, path_server, file_name, servePort, client_addr, client_addr_length);
			t1.detach();
		}
		else
		{
			//从客户端接收文件
			cout << "servePort: " << servePort << endl;
			//get(path_server, file_name, server_socket_fd, client_addr, client_addr_length);
			thread t2(&get, path_server, file_name, servePort, client_addr, client_addr_length);
			t2.detach();
		}
	}
	closesocket(server_socket_fd);
	cout << "EXIT Server!!!\n";
	system("pause");
	return 0;
}
