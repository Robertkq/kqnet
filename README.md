kqnet is a easy-to-use networking library written in C++, internally it uses [asio](https://think-async.com/Asio/) to perform asynchronous communication.

The library implements `client_interface<T>`, `server_interface<T>`, `connection<T>`, `message<T>`.

The template argument `typename T` should be an enum, which role is to define IDs for the messages.

`client_interface<T>` does not require the end user to implement any pure virtual functions.

`server_interface<T>` requires the user to implement the following 5 functions:

```
virtual bool OnClientConnect(connection<T>* client) = 0;
virtual void OnClientDisconnect(connection<T>* client) = 0;
virtual void OnClientValidated(connection<T>* client) = 0;
virtual void OnClientUnvalidated(connection<T>* client) = 0;
virtual void OnMessage(connection<T>* client, message<T>& msg) = 0;
```
