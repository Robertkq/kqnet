#ifndef kqconnection_
#define kqconnection_

#include "common.h"
#include "message.h"
#include "tsqueue.h"

#include "server.h"

namespace kq
{
    template<typename T>
    struct connection
    {
    public:
        enum owner : uint8_t
        {
            client,
            server
        };

        connection() = delete;
        connection(owner parent, asio::io_context& context, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn, uint64_t(*func)(uint64_t), kq::server_interface<T>* serverAddress);
        connection(connection<T>&& other) noexcept;
        virtual ~connection() {}

        connection<T>& operator=(const connection<T>& other) = delete;
        connection<T>& operator=(connection<T>&& other) noexcept;

        uint32_t getID() const { return m_id; }
        asio::ip::tcp::socket::endpoint_type& getIP() { return m_ip; }
        

        void ConnectToClient(uint32_t uid);
        void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints);
        void Disconnect();

        bool IsConnected() const;

        // Send a message to the remote
        void Send(const kq::message<T>& msg);

    private:    

        // Prime context to write a message header
        void WriteHead();

        // Prime context to write a message body
        void WriteBody();

        // Prime context to read a message head.
        void ReadHead();

        // Prime context to read a message body
        void ReadBody();

        void AddToIncomingQueue();

        void WriteValidation();
        void ReadValidation();

        void WriteValidationSuccess();
        void ReadValidationSuccess();
        
        

    protected:
        asio::io_context& m_context; // The context for the socket to work on                           
        asio::ip::tcp::socket m_socket; // Each connection will have a unique socket to the remote
        tsqueue<message<T>> m_qMessagesOut; // Queue holding messages to be sent to remote
        tsqueue<owned_message<T>>& m_qMessagesIn; // Reference to incoming queue of parent object
        message<T> m_msgTemporaryIn; // Auxiliary message for reading
        owner m_ownerType; // A connection behaves differently for a server or client

        uint32_t m_id;

        uint64_t m_ValidateNumberIn;
        uint64_t m_ValidateNumberOut;
        uint64_t m_ValidateNumberCheck;

        uint64_t (*m_scrambleFunc)(uint64_t);
        kq::server_interface<T>* m_serverPtr;
        asio::ip::tcp::socket::endpoint_type m_ip;

        bool m_bValidated;


    }; // end of connection<T>
    
    template<typename T>
    connection<T>::connection(owner parent, asio::io_context& context, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn, uint64_t (*scrambleFunc)(uint64_t), kq::server_interface<T>* serverAddress)
        : m_context(context), m_socket(std::move(socket)), m_qMessagesOut(), m_qMessagesIn(qIn), m_msgTemporaryIn(), m_ownerType(parent), m_id(0),
        m_ValidateNumberIn(0), m_ValidateNumberOut(0), m_ValidateNumberCheck(0), m_scrambleFunc(scrambleFunc), m_serverPtr(serverAddress), m_ip(), m_bValidated(false)
    {
        if (parent == owner::server)
        {
            m_ip = m_socket.remote_endpoint();

            m_ValidateNumberOut = static_cast<uint64_t>(time(NULL));

            m_ValidateNumberCheck = m_scrambleFunc(m_ValidateNumberOut);
        }
    }

    template<typename T>
    connection<T>::connection(connection<T>&& other) noexcept
        : m_context(std::move(other.m_context)), m_socket(std::move(other.m_socket)), m_qMessagesOut(std::move(other.m_qMessagesOut)), m_qMessagesIn(std::move(m_qMessagesIn)),
        m_msgTemporaryIn(std::move(other.m_msgTemporaryIn)), m_ownerType(other.m_ownerType), m_id(other.m_id), m_ValidateNumberIn(other.m_ValidateNumberIn),
        m_ValidateNumberOut(other.m_ValidateNumberOut), m_ValidateNumberCheck(other.m_ValidateNumberCheck), m_scrambleFunc(other.m_scrambleFunc), m_serverPtr(other.m_serverPtr), m_ip(other.m_ip),
        m_bValidated(other.m_bValidated)
    {}

    template<typename T>
    connection<T>& connection<T>::operator=(connection<T>&& other) noexcept
    {
        m_context               = std::move(other.m_context);
        m_socket                = std::move(other.m_socket);
        m_qMessagesOut          = std::move(other.m_qMessagesOut);
        m_qMessagesIn           = std::move(other.m_qMessagesIn);
        m_msgTemporaryIn        = std::move(other.m_msgTemporaryIn);
        m_ownerType             = other.m_ownerType;
        m_id                    = other.m_id;
        m_ValidateNumberIn      = other.m_ValidateNumberIn;
        m_ValidateNumberOut     = other.m_ValidateNumberOut;
        m_ValidateNumberCheck   = other.m_ValidateNumberCheck;
        m_scrambleFunc          = other.m_scrambleFunc;
        m_serverPtr             = other.m_serverPtr;
        m_ip                    = std::move(m_ip);
        other.m_serverPtr       = nullptr; // maybe reconsider ?
        m_bValidated            = other.m_bValidated;
    }

    template<typename T>
    void connection<T>::ConnectToClient(uint32_t uid)
    {
        if (m_ownerType == owner::server)
        {
            if (m_socket.is_open())
            {
                m_ip = m_socket.remote_endpoint();
                m_id = uid;
                // Was: ReadHead();
                // Before starting to read messages, we will validate the client by sending him a number on which he shall perform 
                // a function which the server will check the result from
                WriteValidation();

                ReadValidation();
            }
        }
    }

    template<typename T>
    void connection<T>::ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
    {
        if (m_ownerType == owner::client)
        {
            asio::async_connect(m_socket, endpoints,
                [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                    if (!ec)
                    {
                        m_ip =  m_socket.remote_endpoint();
                        //Was: ReadHead();
                        //Whenever we connect to the server we expect to receive a number for validation
                        ReadValidation();
                        //std::cout << "Connected\n";
                    }
                    else
                    {
                        std::cout << "ConnectToServer() ERROR: " << ec.message() << "\n";
                    }
                });
        }
    }

    template<typename T>
    void connection<T>::Disconnect()
    {
        if (IsConnected())
            asio::post(m_context, [this]() { m_socket.close(); });
    }

    template<typename T>
    bool connection<T>::IsConnected() const
    {
        // A client is "only" connected to the server once it's validated
        return m_socket.m_bValidated;
    }

    

    template<typename T>
    // Send a message to the remote
    void connection<T>::Send(const kq::message<T>& msg)
    {
        asio::post(m_context, [this, msg]() {

            // We assume that if the queue is not empty, it is in the process of sending a message
            bool writing = !m_qMessagesOut.empty();
            // Either way we add the message to the queue.
            m_qMessagesOut.push_back(msg);
            // If we are not sending messages, start sending
            if (writing == false)
            {
                WriteHead();
            }
            });
    }

    template<typename T>
    void connection<T>::WriteHead()
    {
        // We are making a buffer with the first message's head and send it
        asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().head, sizeof(message_header<T>)),
            [this](asio::error_code ec, size_t length) {
                if (!ec)
                {
                    // We sent the message with no problem.
                    if (m_qMessagesOut.front().size() > 0)
                    {
                        // We check if the message has a body and if so, write it
                        WriteBody();
                    }
                    else
                    {
                        // If the message has no body, we are done with it and we can start sending the next one, if available
                        m_qMessagesOut.pop_front();
                        if (m_qMessagesOut.empty() == false)
                        {
                            WriteHead();
                        }
                    }
                }
                else
                {
                    //There was a problem in sending the message
                    std::cout << '[' << m_id << ']' << "WriteHead() ERROR: " << ec.message() << '\n';
                    m_socket.close();
                    if (m_serverPtr != nullptr)
                        return m_serverPtr->__RemoveClient(this);
                }
            });
    }

    template<typename T>
    // Prime context to write a message body
    void connection<T>::WriteBody()
    {
        asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().size()),
            [this](asio::error_code ec, size_t length) {
                if (!ec)
                {
                    // There was no problem in sending the body message, so we are done with this message and can move to the next one.
                    m_qMessagesOut.pop_front();
                    if (m_qMessagesOut.empty() == false)
                    {
                        WriteHead();
                    }
                }
                else
                {
                    // There was a problem in sending the body message.
                    std::cout << '[' << m_id << ']' << "WriteBody() ERROR: " << ec.message() << '\n';
                    m_socket.close();
                    if (m_serverPtr != nullptr)
                        return m_serverPtr->__RemoveClient(this);
                }
            });
    }

    template<typename T>
    // Prime context to read a message head.
    void connection<T>::ReadHead()
    {

        asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.head, sizeof(message_header<T>)),
            [this](std::error_code ec, size_t length) {
                if (!ec)
                {

                    // A message head was successfully read

                    // Check if the message has a body
                    // Note:don't use .size() because we didnt read a body
                    if (m_msgTemporaryIn.head.size > 0)
                    {
                        m_msgTemporaryIn.body.resize(m_msgTemporaryIn.head.size);
                        // We continue reading the message body
                        ReadBody();
                    }
                    else
                    {
                        // We finished reading a whole message, so we add it to the queue of the parent object
                        AddToIncomingQueue();
                    }
                }
                else
                {
                    // There was a problem in reading the message head.
                    std::cout << '[' << m_id << ']' << "ReadHead() ERROR: " << ec.message() << '\n';
                    m_socket.close();
                    if (m_serverPtr != nullptr)
                        return m_serverPtr->__RemoveClient(this);
                }
            });
    }

    template<typename T>
    // Prime context to read a message body
    void connection<T>::ReadBody()
    {
        asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
            [this](asio::error_code ec, size_t length) {
                if (!ec)
                {
                    // We read a message body successfully
                    AddToIncomingQueue();
                }
                else
                {
                    std::cout << '[' << m_id << ']' << "ReadBody() ERROR: " << ec.message() << '\n';
                    m_socket.close();
                    if (m_serverPtr != nullptr)
                        return m_serverPtr->__RemoveClient(this);
                }
            });
    }

    template<typename T>
    void connection<T>::AddToIncomingQueue()
    {
        if (m_ownerType == owner::server)
        {
            m_qMessagesIn.push_back({ this, m_msgTemporaryIn });
        }
        else
        {
            m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });
            // A client doesnt need to know "who" sent the message, it is always the server.
        }

        // After successfully reading a message, start reading the next one.
        ReadHead();

    }

    template<typename T>
    void connection<T>::WriteValidation()
    {

        asio::async_write(m_socket, asio::buffer(&m_ValidateNumberOut, sizeof(uint64_t)),
            [this](asio::error_code ec, size_t length) {
                if (!ec) {
                    //std::cout << "Wrote Validation number\n";
                    if (m_ownerType == owner::client)
                    {

                        // Was: m_bValidated = true; 
                        // Old system used this boolean to confirm validation status, now a message is sent by the server to confirm validation
                        ReadValidationSuccess();
                    }
                }
                else
                {
                    std::cout << '[' << m_id << ']' << "WriteValidation() ERROR: " << ec.message() << '\n';
                    m_socket.close();
                    if (m_serverPtr != nullptr)
                        return m_serverPtr->__RemoveUnvalidatedClient(this);
                }
            });
    }

    

    template<typename T>
    void connection<T>::ReadValidation()
    {
        asio::async_read(m_socket, asio::buffer(&m_ValidateNumberIn, sizeof(uint64_t)),
            [this](asio::error_code ec, size_t length) {
                if (!ec)
                {
                    //std::cout << "Read Validation number\n";
                    if (m_ownerType == owner::client)
                    {
                        // If a client reads a validation, perform the certain function on it and send it back to the server for validation
                        m_ValidateNumberOut = m_scrambleFunc(m_ValidateNumberIn);
                        WriteValidation();
                    }
                    else
                    {
                        // If a server reads a validation, it means the client send it's validation and we check it's validity
                        if (m_ValidateNumberIn == m_ValidateNumberCheck)
                        {
                            // Send a message to the client, informing it that it has been successfully validated;
                            WriteValidationSuccess();
                            if (m_serverPtr != nullptr)
                                m_serverPtr->OnClientValidated(this);

                        }
                        else
                        {
                            m_socket.close();
                            if (m_serverPtr != nullptr)
                                return m_serverPtr->__RemoveUnvalidatedClient(this);
                        }
                    }
                }
                else
                {
                    std::cout << '[' << m_id << ']' << "ReadValidation() ERROR: " << ec.message() << '\n';
                    m_socket.close();
                    if (m_serverPtr != nullptr)
                        return m_serverPtr->__RemoveUnvalidatedClient(this);
                }
            });
    }

    template<typename T>
    void connection<T>::WriteValidationSuccess()
    {
        // Send the validation confirmation to the client
        m_bValidated = true;
        asio::async_write(m_socket, asio::buffer(&m_bValidated, sizeof(bool)),
            [this](asio::error_code ec, size_t length) {
                if (!ec)
                {
                    //std::cout << "Wrote ValidationSuccess\n";
                    if (m_ownerType == owner::server)
                        ReadHead();
                }
                else
                {
                    std::cout << '[' << m_id << ']' << "WriteValidationSuccess() ERROR: " << ec.message() << '\n';
                    m_socket.close();
                }
            });
    }

    template<typename T>
    void connection<T>::ReadValidationSuccess()
    {
        asio::async_read(m_socket, asio::buffer(&m_bValidated, sizeof(bool)),
            [this](asio::error_code ec, size_t length) {
                if (!ec)
                {
                    //std::cout << "Read ValidationSuccess\n";
                    if (m_ownerType == owner::client)
                    {
                        //std::cout << "Read " << length << " bytes\n";
                        // if the client succesfully got this message, we know it's validation it's confirmed
                        // if the client was denied by the server, the connection is closed by the server without sending a message to the client

                        // if the client is validated, the servers sends out a bool with the value true, which is read in    m_bValidated




                        ReadHead();
                    }
                }
                else
                {
                    std::cout << '[' << m_id << ']' << "ReadValidationSuccess() ERROR: " << ec.message() << '\n';
                    m_socket.close();
                }
            });

    }

} // namespace kq

#endif