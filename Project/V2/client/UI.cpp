#include "UI.h"
#include <ctime>
#include <iomanip>
#include <sstream>

UI::UI(Console& console, char* remote_address) :Client(remote_address), console(console)
{
    const auto size = console.get_window_size();
    width = size.first;
    height = size.second;
    screen_buffer.resize(width * height);

#ifdef _DEBUG
    //users.emplace_back("Tom");
    //users.emplace_back("Jave");
    //users.emplace_back("Peter");

    //contents.push_back({ "Tom","Have a nice day!" });
    //contents.push_back({ "Jave", "This is a super loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong string!" });

#endif
}

void UI::draw_border(bool immediate)
{
    std::fill(&location(0, 0), &location(0, width), '*');
    std::fill(&location(height-5, 0), &location(height-5, width), '*');
    if (immediate)
        call_draw();
}
void UI::draw_login(bool immediate)
{
    /*clear the screen*/
    console.clear();
    std::fill(screen_buffer.begin(), screen_buffer.end(), '\0');
    /*draw the login dialog*/
    std::fill(&location(start_row, 10), &location(start_row, 40),'*');
    for(short row=start_row; row<start_row+6; ++row)
    {
        location(row, 10) = '*';
        location(row, 40) = '*';
    }
    std::fill(&location(start_row+6, 10), &location(start_row+6, 40), '*');
    call_draw();
    console.move_cursor_to({ 11, start_row + 2 });
    console << "UserName:>";
    auto current_cursor_pos = console.get_cursor_pos();
    --current_cursor_pos.X;
    console.move_cursor_to({ 11, start_row + 4 });
    console << "Password:";
    console.move_cursor_to(current_cursor_pos);

    //std::fill()
}

void UI::draw_chat(bool immediate)
{
    const auto seperate_line_loc = width - 20;
    short row = 1;
    for (const auto& msg : contents)
    {
        std::copy(msg.user.cbegin(), msg.user.cend(), &location(row, 1)); //ID
        std::stringstream time_s;
        auto time_string = std::put_time(std::localtime(&msg.time), "%c");
        time_s << time_string;
        std::copy(std::istreambuf_iterator{ time_s }, std::istreambuf_iterator<char>{}, & location(row, msg.user.length() + 2));
        location(row, msg.user.length() + 20) = ':';
        ++row;
        //content
        const short rows = std::ceil(static_cast<double>(msg.message.length()) / (seperate_line_loc - 6));
        auto iter = msg.message.cbegin();
        for (size_t i = 0; i < rows; ++row, ++i)
        {
            const auto count = min(std::distance(iter, msg.message.cend()), seperate_line_loc - 6);
            std::copy_n(iter, count, &location(row, 5));
            iter += count;
        }
        ++row;
    }
    if (immediate)
        call_draw();
}

void UI::draw_user(bool immediate)
{
    const auto seperate_line_loc = width - 20;
    const static std::string_view selecter = "[ ]";

    for (short row = 1; row < height - 5; ++row)
    {
        location(row, 0) = '*';
        location(row, seperate_line_loc) = '|';
        if (row - 1 < users.size())
        {
            std::copy(selecter.begin(), selecter.end(), &location(row, seperate_line_loc + 1));
            if (users[row - 1].second)
                location(row, seperate_line_loc + 2) = '+';
            for (short col = seperate_line_loc + selecter.length() + 1, i = 0; col < min(seperate_line_loc + selecter.length() + 1 + users[row - 1].first.length(), width); ++col, ++i)
                location(row, col) = users[row - 1].first[i];
        }

        location(row, width - 1) = '*';
    }
}

void UI::draw_main(bool immediate)
{
    std::fill(screen_buffer.begin(), screen_buffer.end(), '\0');
    draw_border();


    /*draw the chat history*/
    draw_chat();

    /*draw the user section*/
    draw_user();



    call_draw();


    closed = false;
    receiver_thread = std::thread{ [&]
    {
            while (!closed)
            {
                const auto data = get_received_data();
                const auto name_pos = data.find(':');
                auto name = data.substr(0, name_pos);
                auto message = data.substr(name_pos + 2);

                if (name == "Server")
                {
                    if (message.find(',') != std::string::npos)    //process the [who] message
                    {
                        users.clear();
                        std::istringstream userList{ message };
                        std::string user;

                        while (std::getline(userList, user, ','))
                            users.push_back({ user, false });
                        draw_user();
                    }
                }
                contents.push_back({ std::move(name), std::move(message) });
                draw_chat(true);
            }
    } };

    who();  //automatically update the list
    
    do {
        /*move cursor to the button of the screen*/
        console.erase_line();
        console.move_cursor_to({ 0, height - 4 });
        console << '>';

        /*dispatch command here*/
        std::string command;
        std::getline(std::cin, command);
        if (command == "send")
            send();
        else if (command == "who")
            who();
        else if(command=="logout")
        {
            Client::send("logout");
            std::this_thread::sleep_for(std::chrono::seconds{ 1 });
            break;
        }
        else
        {
            console << Color::RED << "Command not found!";
            std::this_thread::sleep_for(std::chrono::seconds{ 1 });
        }
        print_ui_message({});
    } while (true);
}

std::string UI::enter_userID()
{
    std::string userID;
    std::getline(std::cin, userID);
    return userID;
}

std::string UI::enter_passwd()
{
    COORD start_pos{ 11+strlen("password:"), static_cast<short>(start_row + 4) };//length("password:"
    console.move_cursor_to(start_pos);
    console << '>';
    console.move_cursor(1, Direction::LEFT);
    std::string passwd;
    std::getline(std::cin, passwd);
    console.move_cursor_to(start_pos);
    unsigned long written{};
    FillConsoleOutputCharacterA(console.hTerminal, '*', passwd.length(), start_pos, &written);
    return passwd;
}

void UI::send()
{
    console.erase_line();
    console << '>';
    print_ui_message("To who?");

    std::string to;
    std::getline(std::cin, to);
    
    if (to != "all")
    {
        auto user_iter = std::find_if(users.cbegin(), users.cend(), [&to](auto user) {return user.first == to; });
        if (user_iter==users.cend())
            print_ui_message("User [" + to + "] not found!", true);
        else
        {
            /*clear the user selector*/
            for(short i=0; i<users.size(); ++i)
            {
                users[i].second = false;
            }
            users[std::distance(users.cbegin(), user_iter)].second = true;
            draw_user();
            call_draw();
            print_ui_message("Type your message to send");
            console.erase_line();
            console << '>';
            std::string message;
            std::getline(std::cin, message);
            Client::send("send " + to +" "+ message);
        }
    }
    else    //send all
    {
        print_ui_message("Type your message to send");
        console.erase_line();
        console << '>';
        std::string message;
        std::getline(std::cin, message);
        Client::send("send all " + message);
    }
}

void UI::who()
{
    Client::send("who");
}

void UI::print_ui_message(std::string const& message, bool is_warning)
{
    const auto previous_pos = console.get_cursor_pos();
    console.move_cursor(2, Direction::DOWN);
    if (is_warning)
        console << Color::RED;
    console << message;
    console.move_cursor_to(previous_pos);
    console << Color::WHITE;
}

void UI::login()
{
    do {
        draw_login(true);
        auto name = enter_userID();
        auto password = enter_passwd();
        Client::send("login " + name + " " + password);
        auto result = get_received_data();
        if (result.find("Denied") == std::string::npos)
            break;
    } while (true);

    closed = false;
}

std::string UI::get_input()
{
    std::string msg;
    std::getline(std::cin, msg);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    console.move_cursor(1, Direction::UP);
    console.move_cursor(1, Direction::RIGHT);
    console.erase_line();
    console << '>';
    return msg;
}
