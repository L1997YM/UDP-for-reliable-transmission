#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include<cstdlib>
#include<cstring>
#include <string>
#include "fun1.cpp"
#pragma comment(lib, "ws2_32.lib")  //���� ws2_32.dll
#define SERVER_PORT 8000
#define BUFFER_SIZE 51200
#define FILE_NAME_MAX_SIZE 512
using namespace std;
/* ��ͷ */
struct PackInfo
{
	int id;
	int buf_size;
	bool end;
};

/* ���հ� */
struct SendPack
{
	PackInfo head;
	char buf[BUFFER_SIZE];
};
SOCKET & createASocket(int serverPort) {
	//����UDP�׽ӿ�
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT + serverPort);
	//�����׽���
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	//���׽ӿ�
	if (-1 == (MyBind::Bind(serverSocket, server_addr)))
	{
		perror("Server Bind Failed:");
		system("pause");
		exit(1);
	}
	DWORD TIME_OUT = 1000;
	int err = setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&TIME_OUT, sizeof(TIME_OUT));
	if (err != 0) {
		cout << WSAGetLastError() << "\n";
	}
	err = setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&TIME_OUT, sizeof(TIME_OUT));
	if (err != 0) {
		cout << WSAGetLastError() << "\n";
	}
	return serverSocket;
}
void get(char *path, char *file_name, int portNum, struct sockaddr_in server_addr, int server_addr_length)
{
	SOCKET client_socket_fd = createASocket(portNum);
	/*-----------------------�ȴ��ļ����Ϳ�ʼ��Ϣ------------------------------------*/
	char buffer[BUFFER_SIZE];
	memset(buffer, '\0', BUFFER_SIZE);
	while (1) {
		while (recvfrom(client_socket_fd, (char*)&buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &server_addr_length) < 0) {
			sendto(client_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, server_addr_length);
			cout << "Send Request File!!\n" << endl;
		}
		if (strcmp(buffer, "OK!") == 0) {
			cout << "Request accepted!! File Transimission Statrt�� \n";
			break;
		}

	}
	// ���ļ���׼��д�� 
	char* pathfile = strcat(path, file_name);
	std::ofstream ofs(pathfile, ios::binary);
	if (!ofs.is_open())
	{
		printf("File:\t%s Can Not Open To Write\n", pathfile);
		exit(1);
	}
	/*------------------------�ӷ������������ݣ���д���ļ�-----------------------------*/
	PackInfo pack_info;
	pack_info.id = 10;
	SendPack pack_data;
	sendto(client_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&server_addr, server_addr_length);
	while (1)
	{
		if ((recvfrom(client_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&server_addr, &server_addr_length)) > 0)
		{
			if (pack_data.head.id == pack_info.id + 1)
			{
				pack_info.id = pack_data.head.id;
				pack_info.buf_size = pack_data.head.buf_size;
				// �������ݰ�ȷ����Ϣ 
				sendto(client_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&server_addr, server_addr_length);
				// д���ļ� 
				ofs.write(pack_data.buf, sizeof(char) * pack_data.head.buf_size);
				cout << "IDOfPac: " << pack_data.head.id << endl;
				//�ж��ļ������Ƿ����
				if (pack_data.head.end)
				{
					cout << "Get file completed!!\n" << endl;
					break;
				}
			}
			else if (pack_data.head.id == pack_info.id) // ������ط��İ� 
			{
				sendto(client_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&server_addr, server_addr_length);
			}
			else
			{
			}
		}
		else
		{
			if (sendto(client_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&server_addr, server_addr_length) == SOCKET_ERROR)
				cout << "SOCKET_ERROR\n";
			cout << "�ط�pack_info.id:" << pack_info.id << endl;
			//break;
		}
	}
	printf("Receive File:\t%s From Server IP Successful!\n", pathfile);
	closesocket(client_socket_fd);
}
void sent(char *path, char *file_name, int portNum, struct sockaddr_in server_addr, int server_addr_length)
{
	SOCKET client_socket_fd = createASocket(portNum);;
	char* pathfile = strcat(path, file_name);
	int receive_id = 0;
	int send_id = 0;
	// ���ļ� 
	std::ifstream in(pathfile, std::ios::binary);
	if (!in.is_open())
	{
		printf("File:%s Not Found.\n", pathfile);
	}
	else
	{
		printf("File:%s Open Successfully.\n", pathfile);
		PackInfo pack_info;
		char buffer[BUFFER_SIZE];
		memset(buffer, '\0', BUFFER_SIZE);
		strcpy(buffer, "OK!");
		sendto(client_socket_fd, (char*)&buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, server_addr_length);
		while (1) {
			while (recvfrom(client_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&server_addr, &server_addr_length) < 0) {
				sendto(client_socket_fd, (char*)&buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, server_addr_length);
				std::cout << "�ȴ��ͻ���ȷ��\n";
			}

			receive_id = pack_info.id;
			send_id = pack_info.id;
			break; 
		}
		/*-----------------------�����ļ�����-------------------------------------*/
		int num = 0;
		SendPack pack_data;
		pack_data.head.end = false;
		while (1)
		{
			if (receive_id == send_id)
			{
				++send_id;
				if (!in.eof())
				{
					memset(pack_data.buf, '\0', BUFFER_SIZE);
					in.read(pack_data.buf, BUFFER_SIZE * sizeof(char));
					pack_data.head.id = send_id; /* ����id�Ž���ͷ,���ڱ��˳�� */
					pack_data.head.buf_size = in.gcount(); /* ��¼���ݳ��� */
													   //�ļ���ȡ���������ý�����־
					if (in.eof())
					{
						pack_data.head.end = true;
					}
					// �����ļ�����
					sendto(client_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&server_addr, server_addr_length);
					std::cout << "Number: " << num << std::endl;
					num++;
					// ����ȷ����Ϣ 
					while (recvfrom(client_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&server_addr, &server_addr_length) < 0) {
						sendto(client_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&server_addr, server_addr_length);

					};
					std::cout << "pack_info.id��" << pack_info.id << "\n";
					receive_id = pack_info.id;
				}
				else
				{
					break;
				}
			}
			else
			{
				// ������յ�id�ͷ��͵�id����ͬ,���·��� 
				sendto(client_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&server_addr, server_addr_length);
				std::cout << "Number: " << num << std::endl;
				// ����ȷ����Ϣ 
				while (recvfrom(client_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&server_addr, &server_addr_length) < 0) {
					sendto(client_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&server_addr, server_addr_length);
				};
				std::cout << "pack_info.id��" << pack_info.id << "\n";
				receive_id = pack_info.id;
			}
		}
		/* �ر��ļ� */
		in.close();
		printf("File:%s Transfer Successful!\n", pathfile);
		closesocket(client_socket_fd);
	}
}

