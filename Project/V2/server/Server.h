#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <unordered_map>
#include <winsock2.h>
constexpr unsigned short SERVER_PORT = 18112;
constexpr size_t MAX_PENDING = 5;
constexpr size_t MAX_LINE = 256;

constexpr size_t MAX_ID_LENGTH = 32;
constexpr size_t MIN_PASSWD_LENGTH = 4;
constexpr size_t MAX_PASSWD_LENGTH = 8;
constexpr size_t MAX_CLIENTS = 3;

struct Client
{
    int id;
    SOCKET s;
    bool logged_in = false;
    bool is_open = true;
    std::string name;
};

class Server
{
    inline static std::unordered_map<std::string_view, int> commands{ {"login",1}, {"logout",2}, {"send",3}, {"newID",4}, {"sendAll",5},{"who",6}, {"help",7} };
    inline static std::string createUserErrorMsg{ "Error: User id length should be < " + std::to_string(MAX_ID_LENGTH) + " characters. Password should be " + std::to_string(MIN_PASSWD_LENGTH) + " ~ " + std::to_string(MAX_PASSWD_LENGTH) + " long." };

    SOCKET listenSocket;                                //

    std::vector<Client> clients;                        //all the connected client records
    std::unordered_map<std::string, std::string> users; //all the {user, password} record saved on the disk

    /**@brief send raw data -> client, automatically add a "server" tag for server message
     * @param s:    client to send
     * @param data: data to send
     * @param server_msg: if it the message is from server, add a "Server: " tag in front of the message
     */
    void send(Client&s, std::string data, bool sever_msg=false);

    /**@brief: helper function to get the client record
     *@param id: the unique id for the client
     *@returns the iterator pointing to the record
     */
    std::vector<Client>::iterator get_record(int id);

    /**@brief: sync from the client holding by a thread to the client record
     */
    void sync(Client const& to_sync);
public:
    /**@brief: process the accepted client request
     *@param client: the accepted client connection
     *@return true if the processing is done or disconnected
     */
    bool process(Client& client);

    /**@brief: the listening thread and sync with the client record
     *@return the client request with a unique id 
     */
    Client accept();

    /**@brief: load the user and password record on the disk when the server is constructed
     */
    void loadUser();

    /**@brief: process the [login id passwd] command
     *@param client: the accepted client connection
     *@param id: the username
     *@param passwd: the password
     */
    void login(Client& client, std::string id, std::string passwd);

    /**@brief: process the [logout] command
     *@param client: the accepted client connection
     */
    void logout(Client& client);

    /**@brief: process the [send id message] command
     *@param client: the accepted client connection
     *@param msg: the [id message] part
     */
    void message(Client& client, std::string msg);

    /**@breif: process the [who] command
     *@param client: the accepted client connection
     */
    void who(Client& client);

//TODO: refactor here
    /**@brief: process the [send all message] command
     *@param msg: the [message]
     */
    void messageAll(std::string msg, bool sever_msg=false);

    /**@brief: process the [newID id password] command
     *@param client: the accepted client connection
     *@param id: the username
     *@param passwd: the password
     */
    void newID(Client& client, std::string id, std::string passwd);

    /**@brief: send help message
     *@param client: the accepted client connection
     */
    void help(Client& client);

    /**@brief: Construct the server instance, with one thread dedicated for listening on the incoming connection. Create a new thread when a new connection is established
     */
    Server();

    /**@brief: close the listen socket and clean up
     */
    ~Server();
};

