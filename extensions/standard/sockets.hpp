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

#ifdef USE_LIBUV
#include <queue>
#include <mutex>
namespace lamina::net {
    enum SocketType {
        TCP = 0,
        UDP = 1
    };
    enum SocketState {
        CLOSED = 0,
        LISTENING = 1,
        CONNECTED = 2,
        ERR_STATE = 3
    };
    struct SocketError {
        int code;
        std::string message;
    };
    struct Socket {
        uint64_t id;
        SocketType type;
        SocketState state;
        SocketError last_error;
        uv_tcp_t tcp_handle;
        uv_udp_t udp_handle;
        std::queue<std::string> recv_queue;
        std::mutex recv_mutex;
        // 回调
        std::function<void(Socket*, const std::string&)> on_receive;
        // 构造与成员实现
        Socket(uint64_t id, SocketType type = TCP)
            : id(id), type(type), state(SocketState::CLOSED) {
            if (type == TCP) {
                tcp_handle.data = this;
            } else {
                udp_handle.data = this;
            }
        }
        void set_error(int code, const std::string& msg) {
            last_error.code = code;
            last_error.message = msg;
            state = ERR_STATE;
        }
        void queue_data(const std::string& data) {
            std::lock_guard<std::mutex> lock(recv_mutex);
            recv_queue.push(data);
        }
        std::string get_queued_data() {
            std::lock_guard<std::mutex> lock(recv_mutex);
            if (recv_queue.empty()) return "";
            std::string data = recv_queue.front();
            recv_queue.pop();
            return data;
        }
        static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
        static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
    };
    extern uv_loop_t* loop;
    extern uv_tcp_t server;
    extern std::unordered_map<uint64_t, Socket*> clients;
    extern std::atomic<uint64_t> next_client_id;
    extern int64_t on_receive_callback_id;

    int create_socket(uv_stream_t* server, int status);
    void call_lamina_callback(int64_t cbid, uint64_t client_id, const std::string& data);
    Value runnable(const std::vector<Value>& args);
    Value socket_send(const std::vector<Value>& args);
    Value socket_recv(const std::vector<Value>& args);
    Value socket_get_state(const std::vector<Value>& args);
    Value socket_get_error(const std::vector<Value>& args);
    Value socket_register_receive_callback(const std::vector<Value>& args);
    Value socket_close(const std::vector<Value>& args);
    Value socket_connect(const std::vector<Value>& args);
    Value socket_bind(const std::vector<Value>& args);
    Value socket_listen(const std::vector<Value>& args);
    Value socket_accept(const std::vector<Value>& args);
    LAMINA_FUNC("socket_create", runnable, 4);
    LAMINA_FUNC("socket_send", socket_send, 2);
    LAMINA_FUNC("socket_recv", socket_recv, 1);
    LAMINA_FUNC("socket_get_state", socket_get_state, 1);
    LAMINA_FUNC("socket_get_error", socket_get_error, 1);
    LAMINA_FUNC("socket_register_callback", socket_register_receive_callback, 2);
    LAMINA_FUNC("socket_close", socket_close, 1);
    LAMINA_FUNC("socket_connect", socket_connect, 2);
    LAMINA_FUNC("socket_bind", socket_bind, 2);
    LAMINA_FUNC("socket_listen", socket_listen, 2);
    LAMINA_FUNC("socket_accept", socket_accept, 1);
}
#endif

#endif // SOCKETS_HPP
