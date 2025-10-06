#include "sockets.hpp"

#ifdef USE_LIBUV

#include <atomic>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <system_error>
#include <unordered_map>
#include <uv.h>
#include <vector>

namespace lamina::net {
    // 全局变量定义
    uv_loop_t* loop = nullptr;
    std::atomic<uint64_t> next_client_id{1};
    int64_t on_receive_callback_id = 0;
    std::unordered_map<uint64_t, Socket*> clients;
    int64_t server_socket_id = 0;// 记录服务器socket ID
    std::unordered_map<int64_t, std::function<void(uint64_t, const std::string&)>> callback_table;

    Value socket_register_receive_callback(const std::vector<Value>& args) {
        // args[0]: socket_id, args[1]: callback_id
        if (args.size() < 2) {
            L_ERR("socket_register_receive_callback requires 2 arguments (socket_id, callback_id)");
        }
        uint64_t id = static_cast<uint64_t>(args[0].as_number());
        int64_t cbid = static_cast<int64_t>(args[1].as_number());
        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }
        Socket* socket = it->second;
        socket->on_receive = [cbid](Socket* s, const std::string& data) {
            call_lamina_callback(cbid, s->id, data);
        };
        callback_table[cbid] = [cbid](uint64_t client_id, const std::string& data) {
        };
        return Value(0);
    }
    Value socket_bind(const std::vector<Value>& args) {
        // args[0]: socket_id, args[1]: address, args[2]: port
        if (args.size() < 3) {
            L_ERR("socket_bind requires 3 arguments (socket_id, address, port)");
        }
        uint64_t id = static_cast<uint64_t>(args[0].as_number());
        std::string address = args[1].to_string();
        int port = static_cast<int>(args[2].as_number());
        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }
        Socket* socket = it->second;
        sockaddr_in addr;
        uv_ip4_addr(address.c_str(), port, &addr);
        int r = uv_tcp_bind(&socket->tcp_handle, reinterpret_cast<const sockaddr*>(&addr), 0);
        if (r < 0) {
            socket->set_error(r, uv_strerror(r));
            return Value(-1);
        }
        return Value(0);
    }
    Value socket_listen(const std::vector<Value>& args) {
        // args[0]: socket_id, args[1]: backlog
        if (args.size() < 2) {
            L_ERR("socket_listen requires 2 arguments (socket_id, backlog)");
        }
        uint64_t id = static_cast<uint64_t>(args[0].as_number());
        int backlog = static_cast<int>(args[1].as_number());
        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }
        Socket* socket = it->second;
        int r = uv_listen(reinterpret_cast<uv_stream_t*>(&socket->tcp_handle), backlog, [](uv_stream_t* server, int status) {
            create_socket(server, status);
        });
        if (r < 0) {
            socket->set_error(r, uv_strerror(r));
            return Value(-1);
        }
        socket->state = CONNECTED;
        return Value(0);
    }
    Value socket_accept(const std::vector<Value>& args) {
        // args[0]: server_socket_id
        if (args.size() < 1) {
            L_ERR("socket_accept requires 1 argument (server_socket_id)");
        }
        uint64_t id = static_cast<uint64_t>(args[0].as_number());
        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }
        Socket* server = it->second;
        // 这里只能异步等待 create_socket 回调，直接返回 0
        return Value(0);
    }
    void Socket::alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        // libuv 推荐的分配方式
        buf->base = static_cast<char*>(malloc(suggested_size));
        buf->len = static_cast<unsigned int>(suggested_size);
    }

    Value runnable(const std::vector<Value>& args) {
        loop = uv_default_loop();
        uv_run(loop, UV_RUN_DEFAULT);
        return Value(0);
    }
    // 全局变量定义


    // 错误信息映射
    std::unordered_map<int, std::string> error_messages = {
            {UV_EADDRINUSE, "Address already in use"},
            {UV_ECONNREFUSED, "Connection refused"},
            {UV_ECONNRESET, "Connection reset by peer"},
            {UV_ENOTCONN, "Socket is not connected"},
            {UV_EOF, "End of file"},
            {UV_ETIMEDOUT, "Operation timed out"}};


    void call_lamina_callback(int64_t cbid, uint64_t client_id, const std::string& data) {
        auto it = callback_table.find(cbid);
        if (it != callback_table.end()) {
            it->second(client_id, data);
        }
    }
    int create_socket(uv_stream_t* server, int status) {
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

            uint64_t id = next_client_id++;
            Socket* socket = new Socket(id, TCP);

            uv_tcp_init(loop, &socket->tcp_handle);
            uv_tcp_nodelay(&socket->tcp_handle, 1);

            sockaddr_in addr{};
            uv_ip4_addr(address.c_str(), port, &addr);

            int bind_result = uv_tcp_bind(&socket->tcp_handle, reinterpret_cast<const sockaddr*>(&addr), 0);
            if (bind_result < 0) {
                delete socket;
                L_ERR("Bind error: " + std::string(uv_strerror(bind_result)));
            }

            int listen_result = uv_listen(reinterpret_cast<uv_stream_t*>(&socket->tcp_handle), DEFAULT_BACKLOG,
                                          [](uv_stream_t* server, int status) {
                                              create_socket(server, status);
                                          });
            if (listen_result < 0) {
                delete socket;
                L_ERR("Listen error: " + std::string(uv_strerror(listen_result)));
            }

            socket->state = CONNECTED;
            clients[id] = socket;
            server_socket_id = id;

            std::cout << "Server listening on " << address << ":" << port << std::endl;

            // uv_run放到外层或单独启动，避免阻塞这里
            // uv_run(loop, UV_RUN_DEFAULT);

            return Value(static_cast<double>(id));
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

        uint64_t id = (uint64_t) args[0].as_number();
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
            int result = uv_write(req, (uv_stream_t*) &socket->tcp_handle, &buf, 1,
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
                                     (const struct sockaddr*) &addr,
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

        uint64_t id = (uint64_t) args[0].as_number();
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

        uint64_t id = (uint64_t) args[0].as_number();
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

        uint64_t id = (uint64_t) args[0].as_number();
        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }

        Socket* socket = it->second;
        return Value(socket->last_error.message);
    }

    Value socket_connect(const std::vector<Value>& args) {
        if (args.size() < 2) {
            L_ERR("socket_connect requires 2 arguments (address, port)");
        }

        std::string address = args[0].to_string();
        int port = static_cast<int>(args[1].as_number());

        uv_tcp_t* client = new uv_tcp_t;
        uv_tcp_init(loop, client);

        sockaddr_in dest;
        uv_ip4_addr(address.c_str(), port, &dest);

        int result = uv_tcp_connect(new uv_connect_t, client, reinterpret_cast<const sockaddr*>(&dest), [](uv_connect_t* req, int status) {
            if (status < 0) {
                std::cerr << "Connection error: " << uv_strerror(status) << std::endl;
            } else {
                std::cout << "Connected successfully" << std::endl;
            }
            delete req;
        });

        if (result < 0) {
            delete client;
            L_ERR("Connection failed: " + std::string(uv_strerror(result)));
        }

        return Value(0);
    }

    Value socket_close(const std::vector<Value>& args) {
        if (args.size() < 1 || !args[0].is_numeric()) {
            L_ERR("socket_close requires 1 argument (socket_id)");
        }

        uint64_t id = static_cast<uint64_t>(args[0].as_number());
        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }

        Socket* socket = it->second;
        uv_close(reinterpret_cast<uv_handle_t*>(&socket->tcp_handle), [](uv_handle_t* handle) {
            delete static_cast<Socket*>(handle->data);
        });

        clients.erase(it);
        return Value(0);
    }
    static void udp_read_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf,
                            const struct sockaddr* addr, unsigned flags) {
        Socket* socket = static_cast<Socket*>(handle->data);
        if (nread > 0) {
            socket->push_data(buf->base, nread);
            if (socket->on_receive) {
                socket->on_receive(socket, std::string(buf->base, nread));
            }
        } else if (nread < 0) {
            socket->set_error(nread, uv_strerror(nread));
            uv_close((uv_handle_t*) handle, [](uv_handle_t* h) {
                delete static_cast<Socket*>(h->data);
            });
            socket->state = CLOSED;
        }
        delete[] buf->base;
    }

    Value socket_udp_create(const std::vector<Value>& args) {
        if (args.size() < 2) {
            L_ERR("socket_udp_create requires 2 arguments (address, port)");
        }
        std::string address = args[0].to_string();
        int port = static_cast<int>(args[1].as_number());

        uint64_t id = next_client_id++;
        Socket* socket = new Socket(id, UDP);
        uv_udp_init(loop, &socket->udp_handle);

        sockaddr_in addr;
        uv_ip4_addr(address.c_str(), port, &addr);

        int r = uv_udp_bind(&socket->udp_handle, reinterpret_cast<const sockaddr*>(&addr), UV_UDP_REUSEADDR);
        if (r < 0) {
            delete socket;
            L_ERR("UDP bind failed: " + std::string(uv_strerror(r)));
        }

        uv_udp_recv_start(&socket->udp_handle, Socket::alloc_cb, udp_read_cb);


        socket->state = CONNECTED;
        clients[id] = socket;

        return Value(static_cast<double>(id));
    }

    Value socket_udp_send(const std::vector<Value>& args) {
        if (args.size() < 3) {
            L_ERR("socket_udp_send requires 3 arguments (socket_id, address, data)");
        }

        uint64_t id = static_cast<uint64_t>(args[0].as_number());
        std::string address = args[1].to_string();
        std::string data = args[2].to_string();

        auto it = clients.find(id);
        if (it == clients.end()) {
            L_ERR("Invalid socket ID");
        }

        Socket* socket = it->second;
        if (socket->type != UDP) {
            L_ERR("Socket is not UDP");
        }

        sockaddr_in dest;
        uv_ip4_addr(address.c_str(), 0, &dest);// 端口0表示不指定端口，或者需要另外传端口

        uv_buf_t buf = uv_buf_init(const_cast<char*>(data.c_str()), (unsigned int) data.size());

        uv_udp_send_t* send_req = new uv_udp_send_t;
        send_req->data = socket;

        int r = uv_udp_send(send_req, &socket->udp_handle, &buf, 1, reinterpret_cast<const sockaddr*>(&dest),
                            [](uv_udp_send_t* req, int status) {
                                Socket* s = static_cast<Socket*>(req->data);
                                if (status < 0) {
                                    s->set_error(status, uv_strerror(status));
                                }
                                delete req;
                            });

        if (r < 0) {
            socket->set_error(r, uv_strerror(r));
            delete send_req;
            return Value(-1);
        }

        return Value(0);
    }

    void lamina::net::Socket::read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
        Socket* socket = static_cast<Socket*>(stream->data);
        if (nread > 0) {
            socket->incoming_data.push(std::string(buf->base, nread));
            if (socket->on_receive) {
                socket->on_receive(socket, std::string(buf->base, nread));
            }
        } else if (nread < 0) {
            socket->set_error((int) nread, uv_strerror((int) nread));
            uv_close(reinterpret_cast<uv_handle_t*>(stream), nullptr);
        }
        delete[] buf->base;
    }

    // Lamina 函数注册
    // void register_socket_functions(Interpreter& interpreter) {
    //     interpreter.builtin_functions["socket_create"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_create(args);
    //     };
    //     interpreter.builtin_functions["socket_connect"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_connect(args);
    //     };
    //     interpreter.builtin_functions["socket_listen"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_listen(args);
    //     };
    //     interpreter.builtin_functions["socket_accept"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_accept(args);
    //     };
    //     interpreter.builtin_functions["socket_send"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_send(args);
    //     };
    //     interpreter.builtin_functions["socket_recv"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_recv(args);
    //     };
    //     interpreter.builtin_functions["socket_close"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_close(args);
    //     };
    //     interpreter.builtin_functions["socket_get_state"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_get_state(args);
    //     };
    //     interpreter.builtin_functions["socket_get_error"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_get_error(args);
    //     };
    //     interpreter.builtin_functions["socket_register_callback"] = [](const std::vector<Value>& args) -> Value {
    //         return socket_register_receive_callback(args);
    //     };
    // }
}// namespace lamina::net
// namespace lamina::net

#else// USE_LIBUV

Value runnable(const std::vector<Value>& args) {
    // LibUV not available, return error
    std::cerr << "LibUV not available, socket operations not supported." << std::endl;
    return Value(-1);
}

Value socket_send(const std::vector<Value>&) {
    L_ERR("socket_send not supported on this platform.");
    return Value(-1);
}

#endif
