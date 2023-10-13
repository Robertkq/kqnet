#include "common.h"

int main()
{
    kq::client_interface<msgids> client(scramble);
    client.Connect("192.168.100.63", 60000);

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        client.Send(kq::message<msgids>{msgids::Transmitted});

        if(!client.Incoming().empty())
        {
            auto msg = client.Incoming().pop_front().msg;
            std::cout << "I got this message: " << msg.getID() << "\n";
        }
    }
}