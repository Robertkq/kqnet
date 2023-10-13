#include "common.h"

struct testServer : public kq::server_interface<msgids>
{
    testServer(uint16_t port, uint64_t(*scramble)(uint64_t)) : kq::server_interface<msgids>(port, scramble) {}
    ~testServer() {}

    bool OnClientConnect(kq::connection<msgids>* client)
    {
        return true;
    }

    void OnClientDisconnect(kq::connection<msgids>* client)
    {

    }

    void OnClientValidated(kq::connection<msgids>* client)
    {

    }

    void OnClientUnvalidated(kq::connection<msgids>* client)
    {

    }

    void OnMessage(kq::connection<msgids>* client, kq::message<msgids>& msg)
    {
        switch(msg.getID())
        {
            case Transmitted:
                MessageClient(client, kq::message<msgids>{msgids::Received});
            break;

            case Received:
            break;
        }
    }
};

int main()
{
    testServer server(60000, scramble);
    server.Start();

    while(true)
    {
        server.Update();
    }

    return 0;
}

