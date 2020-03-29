#pragma once
#include <string>
#include <winsock2.h>

constexpr unsigned short SERVER_PORT = 18112;
constexpr size_t MAX_LINE = 256;
class Client
{
	SOCKET s;
	
public:
    Client(char* remote_address);
    void send(const std::string& data);
	void receive();

    ~Client()
	{
		closesocket(s);
	}
};

