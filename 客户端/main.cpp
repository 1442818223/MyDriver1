#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
	WSADATA wsaData;
	SOCKET clientSocket;
	sockaddr_in serverAddr;

	// 初始化 Winsock
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建 Socket
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed!" << std::endl;
		WSACleanup();
		return 1;
	}

	// 设置服务器地址
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080); // 服务器监听的端口
	serverAddr.sin_addr.s_addr = inet_addr("192.144.206.247"); // 替换为服务器的公网 IP

	// 连接服务器
	if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Connection failed!" << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Connected to server!" << std::endl;

	// 发送数据
	const char* message = "Hello from client!";
	send(clientSocket, message, strlen(message), 0);

	// 接收服务器的响应
	char recvBuffer[512];
	int bytesReceived = recv(clientSocket, recvBuffer, 512, 0);
	if (bytesReceived > 0) {
		recvBuffer[bytesReceived] = '\0';
		std::cout << "Received from server: " << recvBuffer << std::endl;
	}

	// 关闭 Socket
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}
