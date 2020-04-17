#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <winsock2.h>
constexpr unsigned short SERVER_PORT = 18112;
constexpr size_t MAX_PENDING = 5;
constexpr size_t MAX_LINE = 256;

constexpr size_t MAX_ID_LENGTH = 32;
constexpr size_t MIN_PASSWD_LENGTH = 4;
constexpr size_t MAX_PASSWD_LENGTH = 8;
class Server
{
    inline static std::unordered_map<std::string_view, int> commands{ {"login",1}, {"logout",2}, {"send",3}, {"newID",4}, {"help",5} };
    inline static std::string createUserErrorMsg{ "Error: User id length should be < " + std::to_string(MAX_ID_LENGTH) + " characters. Password should be " + std::to_string(MIN_PASSWD_LENGTH) + " ~ " + std::to_string(MAX_PASSWD_LENGTH) + " long." };
    SOCKET listenSocket;
    SOCKET s;
    std::unordered_map<std::string, std::string> users;
    bool logged_in = false;
    std::string logged_in_user;
    void send(std::string data);
public:
    bool process(); //if logout, return true
    void accept();
    void loadUser();
    void login(std::string id, std::string passwd);
    void logout();
    void message(std::string msg);
    void newID(std::string id, std::string passwd);
    void help();
    Server();
    ~Server();
};

