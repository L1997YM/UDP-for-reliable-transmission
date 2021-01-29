#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include<ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include<cstdlib>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll
#define SERVER_PORT 8000
#define BUFFER_SIZE 51200
#define FILE_NAME_MAX_SIZE 512
using namespace std;
/* 包头 */
struct PackInfo
{
	int id;
	int buf_size;
	bool end;
};

/* 接收包 */
struct SendPack
{
	PackInfo head;
	char buf[BUFFER_SIZE];
};

void get(char *path, char *file_name, SOCKET my_socket_fd, struct sockaddr_in your_addr, int your_addr_length)
{
	/*-----------------------等待文件发送开始信息------------------------------------*/
	char buffer[BUFFER_SIZE];
	memset(buffer, '\0', BUFFER_SIZE);
	while (1) {
		while (recvfrom(my_socket_fd, (char*)&buffer, sizeof(buffer), 0, (struct sockaddr*)&your_addr, &your_addr_length) < 0) {
			sendto(my_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&your_addr, your_addr_length);
			cout << "Send Request File!!\n" << endl;
		}
		if (strcmp(buffer, "OK!") == 0) {
			cout << "Request accepted!! File Transimission Statrt： \n";
			break;
		}

	} 
	// 打开文件，准备写入
	char* pathfile = strcat(path, file_name);
	std::ofstream ofs(pathfile, ios::binary);
	if (!ofs.is_open())
	{
		printf("File:\t%s Can Not Open To Write\n", pathfile);
		exit(1);
	}
	/*------------------------从服务器接收数据，并写入文件-----------------------------*/
	PackInfo pack_info;
	pack_info.id = 10;
	SendPack pack_data;
	sendto(my_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&your_addr, your_addr_length);
	while (1)
	{
		if ((recvfrom(my_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&your_addr, &your_addr_length)) > 0)
		{
			if (pack_data.head.id == pack_info.id + 1)
			{
				pack_info.id = pack_data.head.id;
				pack_info.buf_size = pack_data.head.buf_size;
				// 发送数据包确认信息
				sendto(my_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&your_addr, your_addr_length);
				// 写入文件
				ofs.write(pack_data.buf, sizeof(char) * pack_data.head.buf_size);
				cout << "IDOfPac: " << pack_data.head.id << endl;
				//判断文件传输是否结束
				if (pack_data.head.end)
				{
					cout << "Get file completed!!\n" << endl;
					break;
				}
			}
			else if (pack_data.head.id == pack_info.id) // 如果是重发的包
			{
				sendto(my_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&your_addr, your_addr_length);
			}
			else
			{
			}
		}
		else
		{
			sendto(my_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&your_addr, your_addr_length);
			cout << "重发pack_info.id:" << pack_info.id << endl;
			//break;
		}
	}
	printf("Receive File:\t%s From Server IP Successful!\n", pathfile);
}


void sent(char *path, char *file_name, SOCKET my_socket_fd, struct sockaddr_in your_addr, int your_addr_length)
{
	char* pathfile = strcat(path, file_name);
	int receive_id = 0;
	int send_id = 0;
	// 打开文件
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
		sendto(my_socket_fd, (char*)&buffer, sizeof(buffer), 0, (struct sockaddr*)&your_addr, your_addr_length);
		while (1) {
			while (recvfrom(my_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&your_addr, &your_addr_length) < 0) {
				sendto(my_socket_fd, (char*)&buffer, sizeof(buffer), 0, (struct sockaddr*)&your_addr, your_addr_length);
				std::cout << "等待客户端确认\n";
			}

			receive_id = pack_info.id;
			send_id = pack_info.id;
			break;
		}
		/*-----------------------进行文件传输-------------------------------------*/
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
					pack_data.head.id = send_id; /* 发送id放进包头,用于标记顺序 */
					pack_data.head.buf_size = in.gcount(); /* 记录数据长度 */
													   //文件读取结束，设置结束标志
					if (in.eof())
					{
						pack_data.head.end = true;
					}
					// 发送文件内容
					sendto(my_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&your_addr, your_addr_length);
					std::cout << "Number: " << num << std::endl;
					num++;
					// 接收确认消息
					while (recvfrom(my_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&your_addr, &your_addr_length) < 0) {
						sendto(my_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&your_addr, your_addr_length);

					};
					std::cout << "pack_info.id：" << pack_info.id << "\n";
					receive_id = pack_info.id;
				}
				else
				{
					break;
				}
			}
			else
			{
				// 如果接收的id和发送的id不相同,重新发送
				sendto(my_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&your_addr, your_addr_length);
				std::cout << "Number: " << num << std::endl;
				// 接收确认消息
				while (recvfrom(my_socket_fd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&your_addr, &your_addr_length) < 0) {
					sendto(my_socket_fd, (char*)&pack_data, sizeof(pack_data), 0, (struct sockaddr*)&your_addr, your_addr_length);
				};
				std::cout << "pack_info.id：" << pack_info.id << "\n";
				receive_id = pack_info.id;
			}
		}
		/* 关闭文件 */
		in.close();
		printf("File:%s Transfer Successful!\n", pathfile);
	}
}
