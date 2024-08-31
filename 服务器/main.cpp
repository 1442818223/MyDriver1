#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>

#include <winsock2.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

void HandleClient(SOCKET clientSocket) {
	char recvBuffer[512];

	// 持续接收并发送数据
	while (true) {
		int bytesReceived = recv(clientSocket, recvBuffer, 512, 0);
		if (bytesReceived > 0) {
			recvBuffer[bytesReceived] = '\0';  // 确保字符串结尾
			std::cout << "Received from client: " << recvBuffer << std::endl;

			const char* message = "Hello from server!";
			send(clientSocket, message, strlen(message), 0);
		}
		else if (bytesReceived == 0) {
			std::cout << "Client disconnected." << std::endl;
			break;  // 客户端断开连接
		}
		else {
			std::cerr << "Receive failed!" << std::endl;
			break;  // 发生错误
		}
	}

	// 关闭 Socket
	closesocket(clientSocket);
}

int main() {
	WSADATA wsaData;
	SOCKET serverSocket;
	sockaddr_in serverAddr, clientAddr;
	int clientAddrSize = sizeof(clientAddr);

	// 初始化 Winsock
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建 Socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed!" << std::endl;
		WSACleanup();
		return 1;
	}

	// 绑定 IP 和端口
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);  // 监听的端口号
	serverAddr.sin_addr.s_addr = INADDR_ANY;


	if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Binding failed!" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// 监听连接
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listening failed!" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Server is listening on port 8080..." << std::endl;

	// 持续接受客户端连接
	while (true) {
		SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "Accept failed!" << std::endl;
			continue;  // 继续接受下一个客户端
		}

		std::cout << "Client connected!" << std::endl;

		// 创建新线程来处理客户端通信
		std::thread clientThread(HandleClient, clientSocket);
		clientThread.detach();  // 将线程分离，使其独立运行
	}

	// 关闭服务器 Socket
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
