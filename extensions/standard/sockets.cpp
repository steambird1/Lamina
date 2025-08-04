#include "sockets.hpp"
#include "../../interpreter/interpreter.hpp"

#ifdef USE_LIBUV

#include <uv.h>
#include <unordered_map>
#include <atomic>
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <queue>
#include <mutex>
#include <functional>
#include <memory>
#include <system_error>

namespace lamina::net {

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
    std::function<void(Socket*, const std::string&)> on_receive;

    explicit Socket(uint64_t id, SocketType type) 
        : id(id), type(type), state(CLOSED) {
        if (type == TCP) {
            tcp_handle.data = this;
        } else {
            udp_handle.data = this;
        }
    }

    void set_error(int code, const std::string& msg) {
        last_error.code = code;
        last_error.message = msg;
        state = lamina::net::SocketState::ERR_STATE;
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

    // 实现 alloc_cb
    static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        buf->base = static_cast<char*>(malloc(suggested_size));
        buf->len = suggested_size;
    }

    // 实现 read_cb
    static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
        Socket* socket = static_cast<Socket*>(stream->data);
        if (nread > 0) {
            socket->queue_data(std::string(buf->base, nread));
            if (socket->on_receive) {
                socket->on_receive(socket, std::string(buf->base, nread));
            }
        } else if (nread < 0) {
            socket->set_error(nread, uv_strerror(nread));
        }
        free(buf->base);
    }
};
    // 全局变量定义
    uv_loop_t* loop = nullptr;
    uv_tcp_t server;
    std::atomic<uint64_t> next_client_id{1};
    int64_t on_receive_callback_id = 0;
    std::unordered_map<uint64_t, Socket*> clients;

    // 错误信息映射
    std::unordered_map<int, std::string> error_messages = {
        {UV_EADDRINUSE, "Address already in use"},
        {UV_ECONNREFUSED, "Connection refused"},
        {UV_ECONNRESET, "Connection reset by peer"},
        {UV_ENOTCONN, "Socket is not connected"},
        {UV_EOF, "End of file"},
        {UV_ETIMEDOUT, "Operation timed out"}
    };

// 全局回调表
std::unordered_map<int64_t, std::function<void(uint64_t, const std::string&)>> callback_table;

void call_lamina_callback(int64_t cbid, uint64_t client_id, const std::string& data) {
    auto it = callback_table.find(cbid);
    if (it != callback_table.end()) {
        it->second(client_id, data);
    }
}    int create_socket(uv_stream_t* server, int status) {
        if (status < 0) return -1;

        uint64_t id = next_client_id++;
        Socket* socket = new Socket(id, TCP);
        uv_tcp_init(loop, &socket->tcp_handle);

        if (uv_accept(server, reinterpret_cast<uv_stream_t*>(&socket->tcp_handle)) == 0) {
            uv_read_start(reinterpret_cast<uv_stream_t*>(&socket->tcp_handle),
                          Socket::alloc_cb,
                          Socket::read_cb);
            socket->state = CONNECTED;
            clients[id] = socket;
            std::cout << "New socket connected: ID=" << id << std::endl;
            return 1;
        } else {
            uv_close(reinterpret_cast<uv_handle_t*>(&socket->tcp_handle), [](uv_handle_t* h) {
                delete static_cast<Socket*>(h->data);
            });
            return 0;
        }
    }

    // 实现 socket_create 函数
    Value socket_create(const std::vector<Value>& args) {
        if (args.size() < 4) {
            L_ERR("socket_create requires 4 arguments.");
        }

        std::string address = args[0].to_string();
        int port = static_cast<int>(args[1].as_number());
        int protocol = static_cast<int>(args[2].as_number());
        int type = static_cast<int>(args[3].as_number());

        if (protocol == 4 && type == 0) {
            loop = uv_default_loop();
            uv_tcp_init(loop, &server);
            uv_tcp_nodelay(&server, 1);

            sockaddr_in addr{};
            uv_ip4_addr(address.c_str(), port, &addr);
            uv_tcp_bind(&server, reinterpret_cast<const sockaddr*>(&addr), 0);

            int r = uv_listen(reinterpret_cast<uv_stream_t*>(&server), DEFAULT_BACKLOG,
                              [](uv_stream_t* server, int status) {
                                  create_socket(server, status);
                              });

            if (r) {
                L_ERR("Listen error: " + std::string(uv_strerror(r)));
            }

            std::cout << "Listening on " << address << ":" << port << std::endl;
            uv_run(loop, UV_RUN_DEFAULT);
            return Value(0);
        } else {
            L_ERR("Only IPv4 + TCP is supported currently.");
        }

        return Value(-1);
    }

    Value socket_send(const std::vector<Value>& args) {
        if (args.size() < 2) {
            L_ERR("socket_send requires 2 arguments (socket_id, data)");
        }

        if (!args[0].is_numeric() || !args[1].is_string()) {
            L_ERR("Usage: socket_send(socket_id, string_data)");
        }

        uint64_t id = (uint64_t)args[0].as_number();
        std::string data = args[1].to_string();

        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }

        Socket* socket = it->second;
        if (socket->state != CONNECTED) {
            socket->set_error(-1, "Socket not connected");
            return Value(-1);
        }

        if (socket->type == TCP) {
            uv_buf_t buf = uv_buf_init(const_cast<char*>(data.c_str()), 
                                     static_cast<unsigned int>(data.size()));
            uv_write_t* req = new uv_write_t;
            int result = uv_write(req, (uv_stream_t*)&socket->tcp_handle, &buf, 1,
                [](uv_write_t* req, int status) {
                    if (status < 0) {
                        Socket* socket = static_cast<Socket*>(req->handle->data);
                        socket->set_error(status, uv_strerror(status));
                    }
                    delete req;
                });
            
            if (result < 0) {
                socket->set_error(result, uv_strerror(result));
                delete req;
                return Value(-1);
            }
        } else {
            // UDP发送实现
            uv_buf_t buf = uv_buf_init(const_cast<char*>(data.c_str()), 
                                     static_cast<unsigned int>(data.size()));
            uv_udp_send_t* req = new uv_udp_send_t;
            sockaddr_storage addr;
            int result = uv_udp_send(req, &socket->udp_handle, &buf, 1,
                                   (const struct sockaddr*)&addr,
                [](uv_udp_send_t* req, int status) {
                    if (status < 0) {
                        Socket* socket = static_cast<Socket*>(req->handle->data);
                        socket->set_error(status, uv_strerror(status));
                    }
                    delete req;
                });
            
            if (result < 0) {
                socket->set_error(result, uv_strerror(result));
                delete req;
                return Value(-1);
            }
        }
        
        return Value(0);
    }

    Value socket_recv(const std::vector<Value>& args) {
        if (args.size() < 1 || !args[0].is_numeric()) {
            L_ERR("socket_recv requires 1 argument (socket_id)");
        }

        uint64_t id = (uint64_t)args[0].as_number();
        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }

        Socket* socket = it->second;
        std::string data = socket->get_queued_data();
        return Value(data);
    }

    Value socket_get_state(const std::vector<Value>& args) {
        if (args.size() < 1 || !args[0].is_numeric()) {
            L_ERR("socket_get_state requires 1 argument (socket_id)");
        }

        uint64_t id = (uint64_t)args[0].as_number();
        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }

        return Value(static_cast<int>(it->second->state));
    }

    Value socket_get_error(const std::vector<Value>& args) {
        if (args.size() < 1 || !args[0].is_numeric()) {
            L_ERR("socket_get_error requires 1 argument (socket_id)");
        }

        uint64_t id = (uint64_t)args[0].as_number();
        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }

        Socket* socket = it->second;
        return Value(socket->last_error.message);
    }

    // 供脚本注册回调接口，传入回调id
    Value socket_register_receive_callback(const std::vector<Value>& args) {
        if (args.size() < 2) {
            L_ERR("socket_register_receive_callback requires 2 arguments (socket_id, callback_id)");
        }
        if (!args[0].is_numeric() || !args[1].is_numeric()) {
            L_ERR("Both socket_id and callback_id must be numeric");
        }
        
        uint64_t socket_id = static_cast<uint64_t>(args[0].as_number());
        int64_t callback_id = static_cast<int64_t>(args[1].as_number());
        
        auto it = clients.find(socket_id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }
        
        Socket* socket = it->second;
        socket->on_receive = [callback_id](Socket* s, const std::string& data) {
            auto cb_it = callback_table.find(callback_id);
            if (cb_it != callback_table.end()) {
                cb_it->second(s->id, data);
            }
        };
        
        std::cout << "Registered receive callback for socket " << socket_id 
                  << " with callback id = " << callback_id << std::endl;
        return Value(0);
    }

    // Lamina 函数注册
    void register_socket_functions(Interpreter& interpreter) {
        interpreter.builtin_functions["socket_create"] = [](const std::vector<Value>& args) -> Value {
            return socket_create(args);
        };
        interpreter.builtin_functions["socket_connect"] = [](const std::vector<Value>& args) -> Value {
            return socket_connect(args);
        };
        interpreter.builtin_functions["socket_bind"] = [](const std::vector<Value>& args) -> Value {
            return socket_bind(args);
        };
        interpreter.builtin_functions["socket_listen"] = [](const std::vector<Value>& args) -> Value {
            return socket_listen(args);
        };
        interpreter.builtin_functions["socket_accept"] = [](const std::vector<Value>& args) -> Value {
            return socket_accept(args);
        };
        interpreter.builtin_functions["socket_send"] = [](const std::vector<Value>& args) -> Value {
            return socket_send(args);
        };
        interpreter.builtin_functions["socket_recv"] = [](const std::vector<Value>& args) -> Value {
            return socket_recv(args);
        };
        interpreter.builtin_functions["socket_close"] = [](const std::vector<Value>& args) -> Value {
            return socket_close(args);
        };
        interpreter.builtin_functions["socket_get_state"] = [](const std::vector<Value>& args) -> Value {
            return socket_get_state(args);
        };
        interpreter.builtin_functions["socket_get_error"] = [](const std::vector<Value>& args) -> Value {
            return socket_get_error(args);
        };
        interpreter.builtin_functions["socket_register_callback"] = [](const std::vector<Value>& args) -> Value {
            return socket_register_receive_callback(args);
        };
    }
}
// namespace lamina::net

#else // USE_LIBUV

Value runnable(const std::vector<Value>&) {
    L_ERR("socket_create requires libuv (Windows/Linux only).");
    return Value(-1);
}

Value socket_send(const std::vector<Value>&) {
    L_ERR("socket_send not supported on this platform.");
    return Value(-1);
}

#endif
