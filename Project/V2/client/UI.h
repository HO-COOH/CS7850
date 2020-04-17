#pragma once
#include "Client.h"
#include <string>
#include <vector>
#include <queue>
#include "Console.h"
#include <ctime>
#include <thread>
#include <atomic>
#include <sstream>


struct Message
{
    std::string user;
    std::string message;
    std::time_t time=std::time(nullptr);
};

class UI: Client
{
    std::vector<std::pair<std::string, bool>> users;
    std::deque<Message> contents;   //{id, msg, time}
    Console& console;
    std::vector<char> screen_buffer;
    short width;
    short height;

    short start_row = 5;
    char& location(short row, short col) { return screen_buffer[row * width + col]; };
    void draw_border(bool immediate=false);
    void draw_login(bool immediate = false);
    void draw_chat(bool immediate = false);
    void draw_user(bool immediate = false);
    void call_draw()
    {
        unsigned long count{};
        WriteConsoleOutputCharacterA(console.hTerminal, screen_buffer.data(), width * height, { 0,0 }, &count);
    }
    std::atomic_bool closed = true;
    std::thread receiver_thread;
    std::string enter_userID();
    std::string enter_passwd();

    void send();
    void who();
    void print_ui_message(std::string const& message, bool is_warning=false);
public:
    UI(Console& console, char* remote_address);
    void draw_main(bool immediate = false);

    void login();
    std::string get_input();
    ~UI()
    {
        console.move_cursor_to({ 0, height - 1 });
        closed = true;
        receiver_thread.join();
    }
};

