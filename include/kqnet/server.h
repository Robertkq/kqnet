#ifndef kqserver_
#define kqserver_

#include "common.h"
#include "message.h"
#include "tsqueue.h"
#include "connection.h"

namespace kq
{
    template<typename T>
    struct server_interface
    {
    public:
        
        server_interface(uint16_t port, uint64_t(*scrambleFunc)(uint64_t));
        
        virtual ~server_interface();

        bool Start();

        void Stop();

        void WaitForClientConnection();

        void KickClient(connection<T>* client);
        
        void MessageClient(connection<T>* client, const message<T>& msg);

        // This function will send a message to all clients except the @ignoreClient
        void MessageAllClients(connection<T>* ignoreClient, const message<T>& msg);

        // nMessagesMax is the maximum amount of messages to answer to in the call to Update
        void Update(size_t nMessagesMax = -1);


        

    public:

            // The end user is supposed to implement those functions in a derived type, implementing the desired behaviour.

            // @ client - is a pointer to a connection which is the clients connected to the server
            // @ msg - is a message which holds informations sent from clients to the server and should be responded to

            
            virtual bool OnClientConnect(connection<T>* client) = 0;
            virtual void OnClientDisconnect(connection<T>* client) = 0;
            virtual void OnClientValidated(connection<T>* client) = 0;
            virtual void OnClientUnvalidated(connection<T>* client) = 0;
            virtual void OnMessage(connection<T>* client, message<T>& msg) = 0;
            

    public:
        void __RemoveClient(connection<T>* client);

        void __RemoveUnvalidatedClient(connection<T>* client);


    private:
        // Queues for messages and connections
        tsqueue<owned_message<T>> m_qMessagesIn;
        kq::deque<connection<T>*> m_qConnections;

        // Asio context and it's own thread
        asio::io_context m_context;
        std::thread m_thrContext;

        // Asio acceptor, handles new connections
        asio::ip::tcp::acceptor m_acceptor;

        uint32_t m_id; // ID system for connections

        uint64_t(*m_scrambleFunc)(uint64_t);
        
    }; // end of server_interface

    template<typename T>
    server_interface<T>::server_interface(uint16_t port, uint64_t(*scrambleFunc)(uint64_t))
        : m_qMessagesIn(), m_qConnections(), m_context(), m_thrContext(),
        m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), m_id(1000),
        m_scrambleFunc(scrambleFunc)
    {}

    template<typename T>
    server_interface<T>::~server_interface()
    {
        Stop();
    }

    template<typename T>
    bool server_interface<T>::Start()
    {
        try
        {
            // Prime the context to wait for a new connection
            WaitForClientConnection();

            // After priming the context with work, start the context on it's own thread
            m_thrContext = std::thread([this]() { m_context.run(); });

        }
        catch (std::exception& ec)
        {
            // Something prevented the server from listening
            std::cout << "[Server] Exception: " << ec.what() << "\n";
            return false;
        }
        std::cout << "[Server] Started!\n";
        return true;
    }

    template<typename T>
    void server_interface<T>::Stop()
    {
        for (auto& client : m_qConnections)
            delete client;

        m_context.stop();
        if (m_thrContext.joinable())
            m_thrContext.join();

        std::cout << "[Server] Stopped!\n";
    }

    template<typename T>
    void server_interface<T>::WaitForClientConnection()
    {
        m_acceptor.async_accept(
            [this](asio::error_code ec, asio::ip::tcp::socket socket) {
                if (!ec)
                {
                    // We successfully got a new connection to the server
                    //std::cout << "[Server] New Connection: " << socket.remote_endpoint() << '\n';

                    connection<T>* newconn = new connection<T>(connection<T>::owner::server, m_context, std::move(socket), m_qMessagesIn, m_scrambleFunc, this);
                    // Give the end user the choice to accept or decline certain connections
                    if (OnClientConnect(newconn) == true)
                    {
                        // Connection approved
                        m_qConnections.push_back(newconn);

                        // IMPORTANT: Task the connection's context to wait for bytes to arrive
                        m_qConnections.back()->ConnectToClient(m_id++);

                        //std::cout << "[" << m_qConnections.back()->getID() << "] Connection Approved!\n";
                    }
                    else
                    {
                        //std::cout << "[" << socket.remote_endpoint() << "] Connection Denied!\n";
                    }
                }
                else
                {
                    std::cout << "[Server] New connection ERROR: " << ec.message() << '\n';
                }
                WaitForClientConnection();
            });
    }

    template<typename T>
    void server_interface<T>::KickClient(connection<T>* client)
    {
        __RemoveClient(client);
    }

    template<typename T>
    void server_interface<T>::MessageClient(connection<T>* client, const message<T>& msg)
    {
        if (client != nullptr && client->IsConnected() == true)
        {
            client->Send(msg);
        }
        else
        {
            // If we can't communicate with the client, remove it from the active connections 
            __RemoveClient(client);
        }
    }

    // This function will send a message to all clients except the @ignoreClient
    template<typename T>
    void server_interface<T>::MessageAllClients(connection<T>* ignoreClient, const message<T>& msg)
    {
        bool removeClients = false;

        for (auto& client : m_qConnections)
        {
            if (client != nullptr && client->IsConnected() == true)
            {
                if (client != ignoreClient)
                {
                    client->Send(msg);
                }
            }
            else
            {
                // dont use __RemoveClient() here because it would modify the container while looping it
                // or find a method to use it 
                removeClients = true;
                OnClientDisconnect(client);
                delete client;
                client = nullptr;
            }
        }
        if (removeClients == true)
        {
            std::remove(m_qConnections.begin(), m_qConnections.end(), nullptr);
        }
    }

    // nMessagesMax is the maximum amount of messages to answer to in the call to Update
    template<typename T>
    void server_interface<T>::Update(size_t nMessagesMax)
    {
        size_t nMessagesCount = 0;
        while (nMessagesCount < nMessagesMax && m_qMessagesIn.empty() == false)
        {
            // Get first message in queue
            owned_message<T> msg = m_qMessagesIn.pop_front();
            // Respond to it
            OnMessage(msg.remote, msg.msg);

            ++nMessagesCount;
        }
    }

    template<typename T>
    void server_interface<T>::__RemoveClient(connection<T>* client)
    {
        OnClientDisconnect(client);
        delete client;
        std::remove(m_qConnections.begin(), m_qConnections.end(), client);
    }

    template<typename T>
    void server_interface<T>::__RemoveUnvalidatedClient(connection<T>* client)
    {
        OnClientUnvalidated(client);
        delete client;
        std::remove(m_qConnections.begin(), m_qConnections.end(), client);
    }

}// namespace kq

#endif