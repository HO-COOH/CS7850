#include "Server.h"


int main()
{
    Server s;
    s.loadUser();
    s.accept();
    while (true)
    {
        if(s.process()) //if session closed, accept new socket, else process it
            s.accept();
    }
}


