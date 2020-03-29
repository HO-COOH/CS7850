#include "Client.h"
#include "winsock2.h"
#include <iostream>

void Client::send(const std::string& data)
{
    // Send and receive data.
    ::send(s, data.c_str(), data.length(), 0);


}

void Client::receive()
{
    char data[MAX_LINE];
    int len = recv(s, data, MAX_LINE, 0);
    data[len] = 0;
    std::cout << '>' << data << '\n';
}

Client::Client(char* remote_address)
{
    // Initialize Winsock.
    if (WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData)!= NO_ERROR)
    {
        std::cerr<<"Error at WSAStartup()\n";
        abort();
    }

    unsigned int ip;
    if (isalpha(remote_address[0])) //translate the server name to resolve -> IP
    {
        auto remote = gethostbyname(remote_address);
        if (remote == nullptr)
        {
            std::cerr << "Host not found!\n";
            WSACleanup();
            abort();
        }
        ip = *reinterpret_cast<unsigned long*>(remote->h_addr);
    }
    else    //given IP
        ip = inet_addr(remote_address);

    //make socket
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) 
    {
        std::cerr<<"Error at socket(): " << WSAGetLastError();
        WSACleanup();
        abort();
    }


    //connect to a server
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = htons(SERVER_PORT);
    if (connect(s, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR) 
    {
        std::cerr << "Failed to connect.\n";
        WSACleanup();
        abort();
    }
}
