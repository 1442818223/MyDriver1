#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
	WSADATA wsaData;
	SOCKET clientSocket;
	sockaddr_in serverAddr;

	// ��ʼ�� Winsock
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// ���� Socket
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed!" << std::endl;
		WSACleanup();
		return 1;
	}

	// ���÷�������ַ
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080); // �����������Ķ˿�
	serverAddr.sin_addr.s_addr = inet_addr("192.144.206.247"); // �滻Ϊ�������Ĺ��� IP

	// ���ӷ�����
	if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Connection failed!" << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Connected to server!" << std::endl;

	// ��������
	const char* message = "Hello from client!";
	send(clientSocket, message, strlen(message), 0);

	// ���շ���������Ӧ
	char recvBuffer[512];
	int bytesReceived = recv(clientSocket, recvBuffer, 512, 0);
	if (bytesReceived > 0) {
		recvBuffer[bytesReceived] = '\0';
		std::cout << "Received from server: " << recvBuffer << std::endl;
	}

	// �ر� Socket
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}
