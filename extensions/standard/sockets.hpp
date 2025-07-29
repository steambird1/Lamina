/* 
     TODO： WINDOWS SOCKET
*/
#ifndef SOCKETS_HPP
#define SOCKETS_HPP

#include <iostream>
#include "lamina.hpp"
#ifdef __linux__
#include <uv.h>
uv_loop_t *loop;
uv_tcp_t server;
#endif
#define DEFAULT_BACKLOG 128

static int create_socket(uv_stream_t *server, int status);
Value runnable(const std::vector<Value>& args);

namespace lamina{
     LAMINA_FUNC("socket_create", runnable, 4);
}
#endif //SOCKETS_HPP
