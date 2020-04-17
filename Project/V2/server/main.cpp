#include "Server.h"
#include <future>
#include <iostream>
//#define cout cerr

std::vector<std::thread> threads;
int main()
{
    Server s;
    s.loadUser();
    
    while (true)
    {
        auto client_s=s.accept();
        threads.emplace_back( [&](auto client)
        {
            while (!s.process(client))
                ;
        }, client_s);
    }
}


