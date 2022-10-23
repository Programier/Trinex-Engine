#pragma once
#include <array>
#include <functional>
#include <string>

// Sockets

#define BUFFER_SIZE 1024

#ifdef _WIN32
#include <WinSock2.h>
#include <mstcpip.h>

using SocketLen = int;
using SocketAddress_in = SOCKADDR_IN;
using Socket = SOCKET;
using ka_prop_t = unsigned long;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>


using SocketLen = socklen_t;
using SocketAddress_in = struct sockaddr_in;
using Socket = int;
using ka_prop_t = int;

#endif


class Server
{
private:
#ifdef _WIN32
    WSADATA wsa;
#endif
    Socket _M_socket = {0};
    std::string _M_ip;
    int _M_port;
    std::array<char, BUFFER_SIZE> _M_buffer;
    std::string _M_message;
    SocketAddress_in _M_info;
    SocketLen _M_info_length = sizeof(SocketAddress_in);
    int _M_buffer_length;

    bool _M_is_running = false;

    std::function<void(std::array<char, BUFFER_SIZE>&, int&)> _M_function;


public:
    Server();
    Server(int port, const std::string& ip, const std::function<void(std::array<char, BUFFER_SIZE>&, int&)>& _function);
    Server(const Server& server);
    Server& operator=(const Server& server);

    int port() const;
    Server& port(int _port);
    bool is_running() const;
    const std::array<char, BUFFER_SIZE>& buffer() const;
    std::array<char, BUFFER_SIZE>& buffer();

    const std::string& ip() const;
    Server& ip(const std::string& _ip);


    ~Server();

    Server& start();
    Server& stop();

private:
    Server& receive();
    Server& send();
};
