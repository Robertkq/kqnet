#ifndef kqmessages_
#define kqmessages_

#include "common.h"

namespace kq
{
    // message_header will always have a fixed size holding information about the size of the additional message
    template<typename T>
    struct message_header
    {
        T id;
        size_t size = 0;
    };

    template<typename T>
    struct message
    {
        message_header<T> head;
        std::vector<uint8_t> body;

        size_t size() const { return body.size(); }

        // This operator will allow addition of information into the message | e.g: msg << int(4);
        template<typename dataType>
        message& operator<<(const dataType& value)
        {
            size_t i = size();
            body.resize(size() + sizeof(dataType));
            std::memcpy(body.data() + i, &value, sizeof(dataType));
            head.size = size();

            return *this;
        }

        // This operator will allow reading information from the message | e.g: msg >> foo;
        template<typename dataType>
        message& operator>>(dataType& value)
        {
            size_t i = size() - sizeof(dataType);
            std::memcpy(&value, body.data() + i, sizeof(dataType));
            body.resize(i);
            head.size = size();
            
            return *this;
        }
    };

    // Forward declaring of connection
    template<typename T>
    struct connection;

    // An owned message is just a message paired with a pointer to a connection
    template<typename T>
    struct owned_message
    {
        connection<T>* remote = nullptr;
        message<T> msg;
    };
}

#endif