#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include "fun.h"
#pragma comment(lib, "ws2_32.lib")  //���� ws2_32.dll
#define SERVER_PORT 8000

using namespace std;

int main()
{
	WORD sockVersion = MAKEWORD(2, 2);	//����Winsock  �汾2.2
	WSAData wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) {	//������Ӧ��socket���
		cout << "WSA��ʼ��ʧ��" << WSAGetLastError() << "\n";
		system("pause");
		return -1;
	}

	while (1)
	{
		printf("Please input ins:\"LFTP lsend myserver mylargefile\" \n              or \"LFTP lget myserver mylargefile\"\n");
		int id = 1;
		char ins[512];
		memset(ins, '\0', 512);
		string s;
		getline(std::cin, s);
		if (s[0] == 'e'&&s[1] == 'x'&&s[2] == 'i'&&s[3] == 't')
		{
			break;
		}
		if (s[0] != 'L' || s[1] != 'F' || s[2] != 'T' || s[3] != 'P' || s[4] != ' ' || s[5] != 'l' || (s[6] != 'g' && s[6] != 's'))
		{
			printf("Error ins!\n");
			continue;
		}
		for (int i = 0; i < s.length(); ++i)
		{
			ins[i] = s[i];
		}
		char addr[20];
		memset(addr, '\0', 20);
		char file_name[20];
		memset(file_name, '\0', 20);
		int i = 10;
		if (ins[6] == 's')
		{
			i++;
		}
		for (int j = 0; ins[i] != ' '; ++i, ++j)
		{
			addr[j] = ins[i];
		}
		i++;
		for (int j = 0; i < strlen(ins); ++i, ++j)
		{
			file_name[j] = ins[i];
		}

		/* ����˵�ַ */
		struct sockaddr_in server_addr;
		memset(&server_addr, '0', sizeof(server_addr));
		server_addr.sin_family = AF_INET;						//Э������
		server_addr.sin_addr.S_un.S_addr = inet_addr(addr);     //��������IP��ַ
																//inet_addr��һ�����ʮ���Ƶ�IPת����һ��������������u_long���ͣ�
		server_addr.sin_port = htons(SERVER_PORT);              //��������Ӧ�ó���˿�
		int server_addr_length = sizeof(server_addr);

		/* ����socket */
		SOCKET client_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (client_socket_fd == INVALID_SOCKET)
		{
			perror("Create Socket Failed:");
			exit(1);
		}
		DWORD TIME_OUT = 1000;	//DWORD 32λ�޷�������
		int err = setsockopt(client_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&TIME_OUT, sizeof(TIME_OUT));	//SO_RCVTIMEO���ý������ݳ�ʱʱ�䣬SO_SNDTIMEO���÷������ݳ�ʱʱ��
		if (err != 0) {
			std::cout << WSAGetLastError() << "\n";
		}

		//�����˷���ins
		if (sendto(client_socket_fd, ins, 512, 0, (struct sockaddr*)&server_addr, server_addr_length) == SOCKET_ERROR)
		{
			perror("Send ins Failed:");
			exit(1);
		}
		 
		char path_client[50];
		memset(path_client, '\0', 50);
		string s2 = "C:\\Users\\LYM\\Desktop\\client\\";
		for (int i = 0; i < s2.length(); ++i)
		{
			path_client[i] = s2[i];
		}

		if (ins[6] == 'g')
		{
			get(path_client, file_name, client_socket_fd, server_addr, server_addr_length);
		}
		else
		{
			sent(path_client, file_name, client_socket_fd, server_addr, server_addr_length);
		}
		// �ر��׽���
		closesocket(client_socket_fd);
	}
	// �ر�winsocket��
	WSACleanup();
	return 0;
}
