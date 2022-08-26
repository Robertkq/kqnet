#ifndef kqclient_
#define kqclient_

#include "common.h"
#include "message.h"
#include "tsqueue.h"

namespace kq
{
    template<typename T>
    struct client_interface
    {
    public:
        client_interface(uint64_t(*scrambleFunc)(uint64_t));

        virtual ~client_interface();

        bool Connect(const std::string& host, uint16_t port);
        
        void Disconnect();

        bool IsConnected() const;

        bool IsValidated() const;

        void Send(const message<T>& msg);

        tsqueue<owned_message<T>>& Incoming();



    private:
        asio::io_context m_context;
        std::thread m_thrContext;
        
        connection<T>* m_connection;
        tsqueue<owned_message<T>> m_qMessagesIn;

        uint64_t(*m_scrambleFunc)(uint64_t);
    }; // end of client_interface

    template<typename T>
    client_interface<T>::client_interface(uint64_t(*scrambleFunc)(uint64_t))
        : m_context(), m_thrContext(), m_connection(nullptr), m_qMessagesIn(), m_scrambleFunc(scrambleFunc)
    {} 

    template<typename T>
    client_interface<T>::~client_interface()
    {
        Disconnect();
    }

    template<typename T>
    bool client_interface<T>::Connect(const std::string& host, uint16_t port)
    {
        try
        {
            // Resolve hostname/ip to endpoints
            asio::ip::tcp::resolver resolver(m_context);
            asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

            m_connection = new connection<T>(connection<T>::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qMessagesIn, m_scrambleFunc, nullptr);

            m_connection->ConnectToServer(endpoints);

            m_thrContext = std::thread([this]() { m_context.run(); });

            while (IsValidated() == false)
            {
                // This while loop waits until the client is validated
                // Until the client is not validated, it is not entitled to send messages
                // If the client is not validated, the server will close the connection
            }
        }
        catch (std::exception ec)
        {
            std::cout << "[Client] Connect() ERROR: " << ec.what() << '\n';
            return false;
        }
        return true;
    }

    template<typename T>
    void client_interface<T>::Disconnect()
    {
        if (m_connection->IsConnected())
            m_connection->Disconnect();

        m_context.stop();
        if (m_thrContext.joinable())
            m_thrContext.join();

        delete m_connection;
    }

    template<typename T>
    bool client_interface<T>::IsConnected() const
    {
        return m_connection != nullptr && m_connection->IsConnected() == true;
    }

    template<typename T>
    bool client_interface<T>::IsValidated() const
    {
        return IsConnected() && m_connection->IsValidated();
    }

    template<typename T>
    void client_interface<T>::Send(const message<T>& msg)
    {
        if (IsConnected())
            m_connection->Send(msg);
    }

    template<typename T>
    tsqueue<owned_message<T>>& client_interface<T>::Incoming()
    {
        return m_qMessagesIn;
    }

} // namespace kq

#endif