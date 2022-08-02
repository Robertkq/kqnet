#ifndef kqmessages_
#define kqmessages_

#include "common.h"

namespace kq
{
    // message_header will always have a fixed size holding information about the size of the additional message
    template<typename T>
    struct message_header
    {
    public:
        message_header();
        message_header(T _id);
        message_header(const message_header<T>& other);
        message_header(message_header<T>&& other);

        message_header<T>& operator=(const message_header<T>& other);
        message_header<T>& operator=(message_header<T>&& other);

        T& getID() { return id; }
        const T& getID() const { return id; }
            
        // Was: private: FIXME: Certain functions need access to size / id, add functions to keep them private ?
    public: 
        T id;
        size_t size = 0;
    };

    template<typename T>
    struct message
    {
    public:
        message();
        message(T _id);
        message(const message<T>& other);
        message(message<T>&& other);

        message<T>& operator=(const message<T>& other);
        message<T>& operator=(message<T>&& other);

        const T& getID() const { return head.getID(); }
        T& getID() { return head.getID(); }
        void setID(T newid) const { head.getID() = newid; }
        size_t size() const { return body.size(); }

        // This operator will allow addition of information into the message | e.g: msg << int(4) << bool(false);
        template<typename dataType>
        message<T>& operator<<(const dataType& value);

        // This operator will allow reading information from the message | e.g: msg >> b >> i;
        // Make sure to read the data out in the reverse order that has been sent
        template<typename dataType>
        message<T>& operator>>(dataType& value);

        //Was: private: | FIXME: W/R H/B functions need access to head & body addresses, add functions to keep them private ?
    public:
        message_header<T> head;
        std::vector<uint8_t> body;

    };

    // Forward declaring of connection
    template<typename T>
    struct connection;

    // An owned message is just a message paired with a pointer to a connection
    template<typename T>
    struct owned_message
    {
        // implementation uses automatically generated constructors from compiler
        connection<T>* remote = nullptr;
        message<T> msg;
    };


    template<typename T>
    message_header<T>::message_header()
        : id(), size() {}

    template<typename T>
    message_header<T>::message_header(T _id)
        : id(_id), size() {}

    template<typename T>
    message_header<T>::message_header(const message_header<T>& other)
        : id(other.id), size(other.size) {}

    template<typename T>
    message_header<T>::message_header(message_header<T>&& other)
        : id(other.id), size(other.size) 
    {
        other.size = 0;
    }
            

    template<typename T>
    message_header<T>& message_header<T>::operator=(const message_header<T>& other)
    {
        id = other.id;
        size = other.size;
        return *this;
    }

    template<typename T>
    message_header<T>& message_header<T>::operator=(message_header<T>&& other)
    {
        id = other.id;
        size = other.size;
        other.size = 0;
        //FIXME: consider discarding of id value somehow ?
        return *this;
    }


    template<typename T>
    message<T>::message()
        : head(), body() {}
    

    template<typename T>
    message<T>::message(T id)
        : head(id), body() {}

    template<typename T>
    message<T>::message(const message<T>& other)
        : head(other.head), body(other.body) {}

    template<typename T>
    message<T>::message(message<T>&& other)
        : head(std::move(other.head)), body(std::move(other.body)) {}


    template<typename T>
    message<T>& message<T>::operator=(const message<T>& other)
    {
        head = other.head;
        body = other.body;
        return *this;
    }

    template<typename T>
    message<T>& message<T>::operator=(message<T>&& other)
    {
        head = std::move(other.head);
        body = std::move(other.body);
        return *this;
    }

    template<typename T>
    template<typename dataType>
    message<T>& message<T>::operator<<(const dataType& value)
    {
        size_t i = size();
        body.resize(size() + sizeof(dataType));
        std::memcpy(body.data() + i, &value, sizeof(dataType));
        head.size = size();

        return *this;
    }

    template<typename T>
    template<typename dataType>
    message<T>& message<T>::operator>>(dataType& value)
    {
        size_t i = size() - sizeof(dataType);
        std::memcpy(&value, body.data() + i, sizeof(dataType));
        body.resize(i);
        head.size = size();

        return *this;
    }

}

#endif