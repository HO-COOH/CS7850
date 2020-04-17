#include <iostream>
#include "Client.h"
#include <string>
#include <thread>
#include <atomic>

#include "UI.h"
    HANDLE Console::hTerminal = GetStdHandle(STD_OUTPUT_HANDLE);
    ConsoleWindow Console::console_window;
void fancy_version(int argc, char**argv)
{

    Console console;
    UI ui{ console, argc < 2 ? "127.0.0.1" : argv[1] };
    console.clear();
    ui.login();
    ui.draw_main();
    std::cin.get();
}

void traditional_version(int argc, char** argv)
{
    Client client(argc < 2 ? "127.0.0.1" : argv[1]);
    std::string command;
    std::atomic_bool closed = false;

    std::thread receiver_thread{ [&] {while (!closed) client.receive(); } };
    while (true)
    {
        std::cout << '>';
        std::getline(std::cin, command);
        if (command.empty())
            continue;
        if (command.length() > MAX_LINE - 1)
        {
            std::cerr << "Command too long! Please retry!\n";
            continue;
        }
        client.send(command);
        if (command == "logout")
            break;
    }
    closed = true;
    std::cout << "Session closed!\n";
    receiver_thread.join();
}

int main(int argc, char **argv)
{
#ifndef _DEBUG
    if (argc < 2)
    {
        std::cerr << "\nUsage: client serverName\n";
        return -1;
    }
#endif
    puts("Press 'y' for fancy interactive version or 'n' for traditional version");
    char c{};
    std::cin.get(c);
    std::cin.get(); //get rid of the redundant "Enter" key
    if (c == 'Y' || c == 'y')
        fancy_version(argc, argv);
    else
        traditional_version(argc, argv);
}
