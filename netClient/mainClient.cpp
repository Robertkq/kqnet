#include <iostream>

#include "..\netCommon\kqNet.h"


enum ServerTags : uint8_t
{
    ServerAccept,
    ServerReject,
    RequestAccept,
    MessageRequest,
    MessageSent
};

uint64_t scramble(uint64_t nInput) 
{
    uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
    out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
    return out ^ 0xC0DEFACE12345678;
}

struct client : kq::client_interface<ServerTags>
{
    std::string name;
    client() : kq::client_interface<ServerTags>(&scramble) {}
    
    
};



int main()
{

    client Player;

    Player.name = "Robert";

    Player.Connect("79.118.12.219", 60000);

    while (Player.IsValidated() == false)
    {
        // This while loop waits until the client is validated
        // If the client is not validated, it is not entitled to send messages
        
    }
        
        kq::message<ServerTags> msg;
        msg.head.id = ServerTags::RequestAccept;
        
        Player.Send(msg);
      

        while (true)
        {
            if (Player.IsConnected())
            {
                if (Player.Incoming().empty() == false)
                {
                    auto msg = Player.Incoming().front().msg;
                    Player.Incoming().pop_front();

                    switch (msg.head.id)
                    {
                    case ServerTags::ServerAccept:
                        std::cout << "Server accepted me!\n";
                        break;
                    }
                }
            }
        }


    return 0;
}
