#include "Server.h"


int main()
{
    Server s;
    s.loadUser();
    s.accept();
    while (true)
    {
        if(s.process())
            s.accept();
    }
}


