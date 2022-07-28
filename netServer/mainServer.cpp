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

struct server :  kq::server_interface<ServerTags>
{
    server(uint16_t port, uint64_t(*func)(uint64_t)) : kq::server_interface<ServerTags>(port, func) {}
    ~server() {}
protected:
    
    bool OnClientConnect(kq::connection<ServerTags>* client) override { return true; }
    void OnClientDisconnect(kq::connection<ServerTags>* client) override 
    {
        std::cout << "Succesfully disconnected " << client->getID() << " " << client->getIP() << "\n";
    }
    void OnClientValidated(kq::connection<ServerTags>* client) override 
    {
        std::cout << "Client " << client->getID() << " Validated.\n";
    }
    void OnClientUnvalidated(kq::connection<ServerTags>* client) override
    {
        std::cout << client->getIP() << " UNVALIDATED! \n";
    }
    void OnMessage(kq::connection<ServerTags>* client, kq::message<ServerTags>& msg) override 
    {
        std::cout << "Responding to Client " << client->getID() << ", msg id: " << msg.head.id << ", msg size: " << msg.size() << '\n';
        switch (msg.head.id)
        {
        case(ServerTags::RequestAccept):
            kq::message<ServerTags> answer;
            answer.head.id = ServerTags::ServerAccept;
            client->Send(answer);
            break;
        }
    }
};

int main()
{
    server Server(60000, &scramble);
    Server.Start();
    while (true)
    {
        Server.Update();
    }
    
    return 0;
}