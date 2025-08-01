#ifndef SOCKETS_HPP
#define SOCKETS_HPP

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <atomic>
#include "lamina.hpp"

#ifdef _WIN32
#define USE_LIBUV
#endif

#ifdef __linux__
#define USE_LIBUV
#endif

#ifdef USE_LIBUV
#include <uv.h>
#endif

constexpr int DEFAULT_BACKLOG = 128;

Value runnable(const std::vector<Value>& args);
Value socket_send(const std::vector<Value>& args);

namespace lamina {
    LAMINA_FUNC("socket_create", runnable, 4);
    LAMINA_FUNC("socket_send", socket_send, 2);
}

#ifdef USE_LIBUV
namespace lamina::net {
    extern uv_loop_t* loop;
    extern uv_tcp_t server;
    struct Client;
    extern std::unordered_map<uint64_t, Client*> clients;
    extern std::atomic<uint64_t> next_client_id;
    extern int64_t on_receive_callback_id;

    int create_socket(uv_stream_t* server, int status);

    void call_lamina_callback(int64_t cbid, uint64_t client_id, const std::string& data);
}
#endif

#endif // SOCKETS_HPP
