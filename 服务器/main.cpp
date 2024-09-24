#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>

#include <winsock2.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

void HandleClient(SOCKET clientSocket) {
	char recvBuffer[512];

	// �������ղ���������
	while (true) {
		int bytesReceived = recv(clientSocket, recvBuffer, 512, 0);
		if (bytesReceived > 0) {
			recvBuffer[bytesReceived] = '\0';  // ȷ���ַ�����β
			std::cout << "Received from client: " << recvBuffer << std::endl;

			const char* message = "Hello from server!";
			send(clientSocket, message, strlen(message), 0);
		}
		else if (bytesReceived == 0) {
			std::cout << "Client disconnected." << std::endl;
			break;  // �ͻ��˶Ͽ�����
		}
		else {
			std::cerr << "Receive failed!" << std::endl;
			break;  // ��������
		}
	}

	// �ر� Socket
	closesocket(clientSocket);
}

int main() {
	WSADATA wsaData;
	SOCKET serverSocket;
	sockaddr_in serverAddr, clientAddr;
	int clientAddrSize = sizeof(clientAddr);

	// ��ʼ�� Winsock
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// ���� Socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed!" << std::endl;
		WSACleanup();
		return 1;
	}

	// �� IP �Ͷ˿�
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);  // �����Ķ˿ں�
	serverAddr.sin_addr.s_addr = INADDR_ANY;


	if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Binding failed!" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// ��������
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listening failed!" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Server is listening on port 8080..." << std::endl;

	// �������ܿͻ�������
	while (true) {
		SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "Accept failed!" << std::endl;
			continue;  // ����������һ���ͻ���
		}

		std::cout << "Client connected!" << std::endl;

		// �������߳�������ͻ���ͨ��
		std::thread clientThread(HandleClient, clientSocket);
		clientThread.detach();  // ���̷߳��룬ʹ���������
	}

	// �رշ����� Socket
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
