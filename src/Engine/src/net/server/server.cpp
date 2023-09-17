#include <iostream>
#include <net/server/server.hpp>
#include <unistd.h>

#define CREATE_SERVER_ERROR "Failed to create Server"
#define CREATE_SOCK_ERROR "Failed to create socket"
#define BIND_ERROR "Failed to bind socket"

#define ERROR(msg) std::runtime_error(msg)

#ifdef _WIN32
#define socket_check(socket) (socket != SOCKET_ERROR)
#define bind_socket_check(status) (!status)
#else
#define socket_check(socket) (socket >= 0)
#define bind_socket_check(status) (status >= 0)

#endif

Server::Server() = default;

Server::Server(const Server& server) : _M_ip(server._M_ip), _M_port(server._M_port), _M_function(server._M_function)
{}

Server::Server(int port, const std::string& ip, const std::function<void(Array<char, BUFFER_SIZE>&, int&)>& _function)
    : _M_ip(ip), _M_port(port), _M_function(_function)
{}

int Server::port() const
{
    return _M_port;
}

Server& Server::port(int _port)
{
    stop();
    _M_port = _port;
    return *this;
}

bool Server::is_running() const
{
    return _M_is_running;
}

const std::string& Server::ip() const
{
    return _M_ip;
}

Server& Server::ip(const std::string& _ip)
{
    stop();
    _M_ip = _ip;
    return *this;
}

const Array<char, BUFFER_SIZE>& Server::buffer() const
{
    return _M_buffer;
}

Array<char, BUFFER_SIZE>& Server::buffer()
{
    return _M_buffer;
}

Server& Server::start()
{
    _M_info.sin_family      = AF_INET;
    _M_info.sin_port        = htons(_M_port);
    _M_info.sin_addr.s_addr = inet_addr(_M_ip.c_str());


#ifdef _WIN32
    if (!(WSAStartup(514, &wsa)))
        throw ERROR(CREATE_SERVER_ERROR);
#endif

    _M_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (!socket_check(_M_socket))
        throw ERROR(CREATE_SOCK_ERROR);

    if (!bind_socket_check(bind(_M_socket, (struct sockaddr*) &_M_info, _M_info_length)))
        throw ERROR(BIND_ERROR);

    printf("Server started at: %s:%d\n", inet_ntoa(_M_info.sin_addr), ntohs(_M_info.sin_port));

    // Server loop
    _M_is_running = true;
    while (_M_is_running)
    {
        receive();
        _M_function(_M_buffer, _M_buffer_length);
        send();
    }
    return *this;
}

Server& Server::stop()
{
    _M_is_running = false;
    std::fill(_M_buffer.data(), _M_buffer.data() + BUFFER_SIZE, 0);
    _M_ip.clear();
    _M_port = 0;
    return *this;
}

Server& Server::receive()
{
    _M_buffer_length =
            recvfrom(_M_socket, _M_buffer.data(), BUFFER_SIZE, 0, (struct sockaddr*) &_M_info, &_M_info_length);
    if (!socket_check(_M_buffer_length))
        stop();

    return *this;
}

Server& Server::send()
{
    auto send_result =
            sendto(_M_socket, _M_buffer.data(), _M_buffer_length, 0, (struct sockaddr*) &_M_info, _M_info_length);
    if (!socket_check(send_result))
        std::cout << __PRETTY_FUNCTION__ << ": Failed to send data" << std::endl;

    std::fill(_M_buffer.data(), _M_buffer.data() + BUFFER_SIZE, 0);
    return *this;
}

Server::~Server()
{
#ifdef _WIN32
    WSACleanup();
    closesocket(_M_socket);
#else
    close(_M_socket);
#endif
}

