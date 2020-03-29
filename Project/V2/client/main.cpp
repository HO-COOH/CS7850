#include <iostream>
#include "Client.h"
#include <string>

int main(int argc, char **argv)
{
#ifndef _DEBUG
    if (argc < 2)
    {
        std::cerr << "\nUsage: client serverName\n";
        return -1;
    }
#endif

    Client client(argc<2? "127.0.0.1" : argv[1]);
    std::string command;

    while(true)
    {
        std::cout << '>';
        std::getline(std::cin, command);
        if(command.length() > MAX_LINE - 1)
        {
            std::cerr << "Command too long! Please retry!\n";
            continue;
        }

        client.send(command);
        client.receive();
        if (command == "logout")
            break;
    }

    std::cout << "Session closed!\n";
}
