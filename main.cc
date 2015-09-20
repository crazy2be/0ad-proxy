#ifdef WINDOWS
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
typedef int WSADATA;
typedef int DWORD;
typedef int SOCKET;
// Sketch I don't thing these are right
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define SOCKADDR_IN sockaddr_in
#define SOCKADDR sockaddr
#define WSAStartup(...) 0
#define WSACleanup(...)
#define ZeroMemory(addr, n) memset(addr, 0, n)
#define closesocket(sock) close(sock)
#endif
#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::string;

inline string to_string(int num) {
	if (num == 0) return "0";

	string str = "";
	bool negative = false;
	if (num < 0) {
		negative = true;
		num = -num;
	}

	while (num > 0) {
		int lastDigit = num % 10;
		num /= 10;
		str = (char)(lastDigit + 48) + str;
	}

	if (negative) {
		str = "-" + str;
	}

	return str;
}

SOCKADDR http_socket_lookup_addr(string host, int port) {
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;          // We are targeting IPv4
	hints.ai_protocol = IPPROTO_TCP;    // We are targeting TCP
	hints.ai_socktype = SOCK_STREAM;    // We are targeting TCP so its SOCK_STREAM

	struct addrinfo* targetAdressInfo = NULL;
	DWORD getAddrRes = getaddrinfo("localhost", NULL, &hints, &targetAdressInfo);
	if (getAddrRes != 0 || targetAdressInfo == NULL) {
		cout << "Could not resolve the Host Name" << endl;
		throw "";
	}

	SOCKADDR_IN sockAddr;
	sockAddr.sin_addr = ((SOCKADDR_IN*) targetAdressInfo->ai_addr)->sin_addr;
	sockAddr.sin_family = AF_INET;    // IPv4
	sockAddr.sin_port = htons(port);

	freeaddrinfo(targetAdressInfo);
	return *((SOCKADDR*)&sockAddr);
}

SOCKET http_socket_open(SOCKADDR* addr) {
	SOCKET webSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (webSocket == INVALID_SOCKET) {
		cout << "Creation of the Socket Failed" << endl;
		return -1;
	}

	//cout << "Connecting..." << endl;
	if (connect(webSocket, addr, sizeof(*addr)) != 0) {
		cout << "Could not connect" << endl;
		closesocket(webSocket);
		return -1;
	}
	//cout << "Connected." << endl;
	return webSocket;
}

void http_socket_request(int webSocket, string method, string path, string data) {
	string httpRequest = \
		method + " " + path + " HTTP/1.0\r\n"\
		"Content-Length: " + to_string(data.size()) + "\r\n"
		"\r\n" +
		data;
	//cout << httpRequest << endl;
	int sentBytes = send(webSocket, httpRequest.c_str(), httpRequest.size(), 0);
	if (sentBytes < httpRequest.size() || sentBytes == SOCKET_ERROR) {
		cout << "Could not send the request to the Server" << endl;
		return;
	}

	// Receiving and Displaying an answer from the Web Server
// 	char buffer[10000];
// 	ZeroMemory(buffer, sizeof(buffer));
// 	int dataLen;
// 	while ((dataLen = recv(webSocket, buffer, sizeof(buffer), 0) > 0)) {
// 		int i = 0;
// 		while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {
// //			cout << buffer[i];
// 			i += 1;
// 		}
// 	}
}

long long usec() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec + tv.tv_sec*1000000;
}

int main() {
	// Initialize Dependencies to the Windows Socket.
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
		cout << "WSAStartup failed." << endl;
		system("pause");
		return -1;
	}

	SOCKADDR addr = http_socket_lookup_addr("localhost", 2500);
	for (int i = 0; i < 10000; i++) {
		long long start = usec();
		SOCKET webSocket = http_socket_open(&addr);
		if (webSocket < 0) {
			system("pause");
			WSACleanup();
			return -1;
		}

		http_socket_request(webSocket, "PUT", "/quentin/"+to_string(i)+".json",
			"{\"pro\": true, \"time\": {\".sv\": \"timestamp\"}}");
		closesocket(webSocket);
		long long end = usec();
		std::cout << "Took " << (double)(end - start)/1000000 << " usec" << endl;
		//sleep(1);
	}

	// Cleaning up Windows Socket Dependencies
	WSACleanup();

	system("pause");
	return 0;
}
