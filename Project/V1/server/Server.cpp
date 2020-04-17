#include "Server.h"
#include <iostream>
#include <fstream>
#include <algorithm>

void Server::send(std::string data)
{
    ::send(s, data.c_str(), data.length(), 0);
}
bool Server::process()
{
    char buf[MAX_LINE];
    auto length = recv(s, buf, MAX_LINE, 0);
    if(length==-1 || s==SOCKET_ERROR)
    {
        closesocket(s);
        std::cout << "Connection closed!\n";
        return true;
    }
    buf[length] = '\0';
    size_t pos = std::distance(buf, std::find(buf, buf + MAX_LINE, ' '));
    const std::string_view command{ buf, pos==MAX_LINE? length:pos};
    if (auto iter = commands.find(command); iter != commands.cend())
    {
        switch (iter->second)
        {
        case 1:
            if (pos == MAX_LINE)
                help();
            else
                login({ buf + pos + 1, std::find(buf+pos+1, buf+MAX_LINE, ' ')}, { &*std::find(std::rbegin(buf), std::rend(buf), ' ')+1});
            break;
        case 2:
            logout();
            return true;
        case 3:
            message({ buf + pos + 1 });
            break;
        case 4:
            {
                if (pos != MAX_LINE)
                {
                    if (const auto id_end = std::find(buf + pos + 1, buf + MAX_LINE, ' '), passwd_start = &*std::find(id_end, buf + MAX_LINE, ' ')+1; id_end != buf + MAX_LINE && passwd_start != buf + MAX_LINE)
                    {
                        newID({ buf + pos + 1, id_end }, { passwd_start });
                        break;
                    }
                }
                else
                    send("Format: newID [username] [passwd]");
                break;
            }
        default:
            help();
        }
    }
    else
        send("Command not found!");
    return false;
}
void Server::accept()
{
    s = ::accept(listenSocket, nullptr, nullptr);
    if (s == SOCKET_ERROR)
    {
        std::cerr << "accept() error \n";
        closesocket(listenSocket);
        WSACleanup();
    }
    std::cout << "Client conncected!\n";
}
void Server::loadUser()
{
    if(std::ifstream file{ "users.txt" }; file.is_open())
    {
        std::string id;
        std::string passwd;
        while (std::getline(file, id, ','))
        {
            std::getline(file, passwd);
            users[id] = passwd;
        }
    }
#ifdef _DEBUG
    std::cout << "User lists:\n";
    for (const auto& p : users)
        std::cout << p.first << "," << p.second << '\n';
#endif // DEBUG

}
void Server::login(std::string id, std::string passwd)
{
    if (auto iter = users.find(id); iter == users.cend() || iter->second != passwd)    //user id not found or passwd incorrect
        send("Server: Denied.");
    else    //success
    {
        logged_in = true;
        logged_in_user = id;
        send("Server: "+ id + " joins");
        std::cout << id << " login\n";
    }
}
void Server::logout()
{
    if (logged_in) //success
    {
        logged_in = false;
        send("Server: " + logged_in_user + "left.");
        std::cout << logged_in_user << " logout.\n";
        logged_in_user.clear();
    }
    else
        send("Server: Denied. Please login first.");
}
void Server::message(std::string msg)
{
    if (logged_in)
    {
        send(logged_in_user + ": " + msg);
        std::cout << logged_in_user << ": " << msg << '\n';
    }
    else
        send("Server: Denied. Please login first.");
}
void Server::newID(std::string id, std::string passwd)
{
#ifdef _DEBUG
    std::cout << "Creating new id: " << id << " password: " << passwd << '\n';
#endif // _DEBUG

    if(id.length()>MAX_ID_LENGTH || passwd.length() < MIN_PASSWD_LENGTH || passwd.length() > MAX_PASSWD_LENGTH) //rules not satisfied
        send(createUserErrorMsg);
    else
    {
        if (users.find(id) != users.cend())
            send("User " + id + " already exist!");
        else
        {
            users[id] = passwd;
            if (std::ofstream file{ "users.txt",std::ios_base::app }; file.is_open())
                file << "\n" << id << ',' << passwd;
            send("Successfully created new user id:" + id);
        }
    }
}
void Server::help()
{
    send("Server: Supported commands:\n\tlogin [username] [password]\n\tlogout\n\tsend [message]\n\tnewID [newUserName] [password]\n\thelp:display this message again");
}
Server::Server()
{
    //initialize winsock
     if(WSADATA wsaData; WSAStartup(MAKEWORD(2,2), &wsaData)!=NO_ERROR)
     {
         std::cerr << "Error at WSAStartup()\n";
         abort();
     }

     //create socket
     listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     if (listenSocket == INVALID_SOCKET) 
     {
         std::cerr<<"Error at socket():" << WSAGetLastError();
         WSACleanup();
         abort();
     }

     //bind the socket
     sockaddr_in addr{};
     addr.sin_family = AF_INET;
     addr.sin_addr.s_addr = INADDR_ANY; //use local address
     addr.sin_port = htons(SERVER_PORT);
     if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR) 
     {
         std::cerr << "bind() failed.\n";
         closesocket(listenSocket);
         WSACleanup();
         abort();
     }

    //listen on the socket
     if (listen(listenSocket, MAX_PENDING) == SOCKET_ERROR) 
     {
         std::cerr << "Error listening on socket.\n";
         closesocket(listenSocket);
         WSACleanup();
         abort();
     }

     std::cout << "Waiting for a client to connect...\n";
}

Server::~Server()
{
    closesocket(listenSocket);
}
