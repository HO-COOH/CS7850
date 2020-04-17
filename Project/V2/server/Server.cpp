#include "Server.h"
#include <iostream>
#include <fstream>
#include <algorithm>

void Server::send(Client& s, std::string data, bool server_msg)
{
    const std::string to_send = server_msg ? "Server: " + data : data;
    ::send(s.s, to_send.c_str(), to_send.length(), 0);
}
std::vector<Client>::iterator Server::get_record(int id)
{
    return std::find_if(clients.begin(), clients.end(), [id](Client& record) {return record.id == id; });
}
void Server::sync(Client const& to_sync)
{
    *get_record(to_sync.id) = to_sync;
}
bool Server::process(Client& client)
{
#ifdef _DEBUG
    std::cerr << "id=" << client.id << '\n';
#endif
    char buf[MAX_LINE]{};
    const auto length = recv(client.s, buf, MAX_LINE, 0);
    if(length==-1 || client.s==SOCKET_ERROR)   //connection closed
    {
        closesocket(client.s);
        clients.erase(get_record(client.id));
        std::cerr << "Connection closed"<<client.id<<"\n";
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
            login(client,{ buf + pos + 1, std::find(buf+pos+1, buf+MAX_LINE, ' ')}, { &*std::find(std::rbegin(buf), std::rend(buf), ' ')+1});
            break;
        case 2:
            logout(client);
            clients.erase(get_record(client.id));
            return true;
        case 3:
            message(client, { buf + pos + 1 });
            break;
        case 4:
            {
                if (pos != MAX_LINE)
                {
                    if (const auto id_end = std::find(buf + pos + 1, buf + MAX_LINE, ' '), passwd_start = &*std::find(id_end, buf + MAX_LINE, ' ')+1; id_end != buf + MAX_LINE && passwd_start != buf + MAX_LINE)
                    {
                        newID(client, { buf + pos + 1, id_end }, { passwd_start });
                        break;
                    }
                }
                else
                    send(client, "Format: newID [username] [passwd]", true);
                break;
            }
        case 5:
            messageAll({ buf + pos + 1 });
            break;
        case 6:
            who(client);
            break;
        case 7:
            help(client);
        }
    }
    else
        send(client, "Command not found!", true);
    sync(client);
    return false;
}
Client Server::accept()
{
    static int id = 0;
    Client temp{ id++, ::accept(listenSocket, nullptr, nullptr),{} };
    if (temp.s == SOCKET_ERROR)
    {
        std::cerr << "accept() error \n";
        closesocket(listenSocket);
        WSACleanup();
        return {};
    }
    std::cout << "Client connected!\n";
    clients.push_back(temp);
    return temp;
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
void Server::login(Client& client, std::string id, std::string passwd)
{
    if (auto iter = users.find(id); iter == users.cend() || iter->second != passwd)    //user id not found or password incorrect
        send(client,"Denied.", true);
    else    //success
    {
        client.logged_in = true;
        sync(client);
        messageAll(id + " joins", true);
        std::cout << id << " login\n";
        client.name = std::move(id);
    }
}
void Server::logout(Client& client)
{
    if (client.logged_in) //success
    {
        send(client, "You successfully logged out!", true);
        client.logged_in = false;
        messageAll(client.name + " left.", true);
        std::cout << client.name << " logout.";
    }
    else
        send(client,  "Denied. Please login first.", true);
}
void Server::message(Client& client, std::string msg)
{
    if (client.logged_in)
    {
        const auto dest_client_name_pos = msg.find(' ');
        auto dest_client_name = msg.substr(0, dest_client_name_pos);
        if (dest_client_name == "all")
            messageAll(client.name+": "+msg.substr(dest_client_name_pos+1));
        else
        {
            if (auto dest_iter = std::find_if(clients.begin(), clients.end(), [&dest_client_name](Client& client) {return client.name == dest_client_name; }); dest_iter != clients.end()) {
                send(*dest_iter, client.name + ": " + msg.substr(dest_client_name_pos+1));
                std::cout << client.name << "->" << msg << '\n';
            }
            else
                send(client, "User [" + dest_client_name + "] not found", true);
        }
    }
    else
        send(client, "Denied. Please login first.", true);
}
void Server::who(Client& client)
{
    if (client.logged_in)
    {
        std::string user_names;
        for (const auto& client : clients)
        {
            if (client.logged_in)
                user_names += (client.name + ",");
        }
        send(client, std::move(user_names), true);
    }
    else
        send(client, "Denied. Please login first.", true);
}
void Server::messageAll(std::string msg, bool server_msg)
{
    for(auto& client:clients)
    {
        if (client.logged_in)
            send(client, msg, server_msg);
    }
}
void Server::newID(Client& client, std::string id, std::string passwd)
{
#ifdef _DEBUG
    std::cout << "Creating new id: " << id << " password: " << passwd << '\n';
#endif // _DEBUG

    if(id.length()>MAX_ID_LENGTH || passwd.length() < MIN_PASSWD_LENGTH || passwd.length() > MAX_PASSWD_LENGTH) //rules not satisfied
        send(client, createUserErrorMsg);
    else
    {
        if (users.find(id) != users.cend())
            send(client, "User " + id + " already exist!");
        else
        {
            users[id] = passwd;
            if (std::ofstream file{ "users.txt",std::ios_base::app }; file.is_open())
                file << "\n" << id << ',' << passwd;
            send(client, "Successfully created new user id:" + id);
        }
    }
}
void Server::help(Client& client)
{
    send(client, "Supported commands : \n\tlogin[username][password]\n\tlogout\n\tsend[userID][message]\n\tsend all[message]\n\tnewID[newUserName][password]\n\twho\n\thelp:display this message again", true);
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
