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
#define STRFY(X) #X
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

int main () {
    // Initialize Dependencies to the Windows Socket.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        cout << "WSAStartup failed.\n";
        system("pause");
        return -1;
    }

    // We first prepare some "hints" for the "getaddrinfo" function
    // to tell it, that we are looking for a IPv4 TCP Connection.
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;          // We are targeting IPv4
    hints.ai_protocol = IPPROTO_TCP;    // We are targeting TCP
    hints.ai_socktype = SOCK_STREAM;    // We are targeting TCP so its SOCK_STREAM

    // Aquiring of the IPv4 address of a host using the newer
    // "getaddrinfo" function which outdated "gethostbyname".
    // It will search for IPv4 addresses using the TCP-Protocol.
    struct addrinfo* targetAdressInfo = NULL;
    DWORD getAddrRes = getaddrinfo("localhost", NULL, &hints, &targetAdressInfo);
    if (getAddrRes != 0 || targetAdressInfo == NULL) {
        cout << "Could not resolve the Host Name" << endl;
        system("pause");
        WSACleanup();
        return -1;
    }

    // Create the Socket Address Informations, using IPv4
    // We dont have to take care of sin_zero, it is only used to extend the length of SOCKADDR_IN to the size of SOCKADDR
    SOCKADDR_IN sockAddr;
    sockAddr.sin_addr = ((SOCKADDR_IN*) targetAdressInfo->ai_addr)->sin_addr;    // The IPv4 Address from the Address Resolution Result
    sockAddr.sin_family = AF_INET;  // IPv4
    sockAddr.sin_port = htons(2500);  // Port for proxy // HTTP Port: 80

    // We have to free the Address-Information from getaddrinfo again
    freeaddrinfo(targetAdressInfo);

    // Creation of a socket for the communication with the Web Server,
    // using IPv4 and the TCP-Protocol
    SOCKET webSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (webSocket == INVALID_SOCKET)
    {
        cout << "Creation of the Socket Failed" << endl;
        system("pause");
        WSACleanup();
        return -1;
    }

    // Establishing a connection to the web Socket
    cout << "Connecting...\n";
    if(connect(webSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) != 0)
    {
        cout << "Could not connect";
        system("pause");
        closesocket(webSocket);
        WSACleanup();
        return -1;
    }
    cout << "Connected.\n";

	string httpData = "{ \"pro\": true, \"time\": {\".sv\": \"timestamp\"} }";
    string httpRequest = "PUT /quentin.json HTTP/1.0\r\nConnection: close\r\nContent-Length: " + to_string(httpData.size()) + "\r\n\r\n" + httpData;
	cout << httpRequest << endl;
    int sentBytes = send(webSocket, httpRequest.c_str(), httpRequest.size(), 0);
    if (sentBytes < httpRequest.size() || sentBytes == SOCKET_ERROR)
    {
        cout << "Could not send the request to the Server" << endl;
        system("pause");
        closesocket(webSocket);
        WSACleanup();
        return -1;
    }

    // Receiving and Displaying an answer from the Web Server
    char buffer[10000];
    ZeroMemory(buffer, sizeof(buffer));
    int dataLen;
    while ((dataLen = recv(webSocket, buffer, sizeof(buffer), 0) > 0))
    {
        int i = 0;
        while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {
            cout << buffer[i];
            i += 1;
        }
    }

    // Cleaning up Windows Socket Dependencies
    closesocket(webSocket);
    WSACleanup();

    system("pause");
    return 0;
}
