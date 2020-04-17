#pragma once
#include <string>
#include <winsock2.h>

constexpr unsigned short SERVER_PORT = 18112;
constexpr size_t MAX_LINE = 256;
class Client
{
	SOCKET s;
	
public:
	/**@brief: Construct the client instances
	 * @param remote_address: the IP address of server to connect to
	 */
    Client(char* remote_address);

	/**@brief: send raw data to the server
	 * @param data: data to send
	 */
    void send(const std::string& data);

	/**@brief: receive raw data from the server and print on the screen, should run in a separate thread as a demon
	 */
	void receive();
	
	/**@brief: get the data received from the server and returns as a string, should run in a separate thread
	 */
	std::string get_received_data();

	
    ~Client()
	{
		closesocket(s);
	}
};

