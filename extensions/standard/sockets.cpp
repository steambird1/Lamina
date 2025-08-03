#include "sockets.hpp"

#ifdef USE_LIBUV

#include <uv.h>
#include <unordered_map>
#include <atomic>
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>

namespace lamina::net {
    uv_loop_t* loop = nullptr;
    uv_tcp_t server;

    std::unordered_map<uint64_t, struct Client*> clients;
    std::atomic<uint64_t> next_client_id{1};

    // 这里保存注册的脚本回调ID，0表示未注册
    int64_t on_receive_callback_id = 0;

    struct Client {
        uint64_t id;
        uv_tcp_t handle;
        uv_write_t write_req;
        std::vector<char> write_buffer;

        explicit Client(uint64_t id) : id(id) {
            handle.data = this;
        }

        void send(const std::string& data) {
            write_buffer.assign(data.begin(), data.end());
            uv_buf_t buf = uv_buf_init(write_buffer.data(), static_cast<unsigned int>(write_buffer.size()));
            uv_write(&write_req, reinterpret_cast<uv_stream_t*>(&handle), &buf, 1, nullptr);
        }

        static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
            *buf = uv_buf_init(static_cast<char*>(malloc(suggested_size)), static_cast<unsigned int>(suggested_size));
        }

        static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
            Client* client = static_cast<Client*>(stream->data);
            if (nread > 0) {
                std::string received(buf->base, nread);
                std::cout << "[recv@" << client->id << "]: " << received << std::endl;

                if (on_receive_callback_id != 0) {
                    call_lamina_callback(on_receive_callback_id, client->id, received);
                }
            } else {
                uv_close(reinterpret_cast<uv_handle_t*>(stream), [](uv_handle_t* h) {
                    auto* c = static_cast<Client*>(h->data);
                    std::cout << "Client disconnected: ID=" << c->id << std::endl;
                    clients.erase(c->id);
                    delete c;
                });
            }
            free(buf->base);
        }
    };

    int create_socket(uv_stream_t* server, int status) {
        if (status < 0) return -1;

        uint64_t id = next_client_id++;
        Client* client = new Client(id);
        uv_tcp_init(loop, &client->handle);

        if (uv_accept(server, reinterpret_cast<uv_stream_t*>(&client->handle)) == 0) {
            uv_read_start(reinterpret_cast<uv_stream_t*>(&client->handle),
                          Client::alloc_cb,
                          Client::read_cb);
            clients[id] = client;
            std::cout << "New client connected: ID=" << id << std::endl;
            return 1;
        } else {
            uv_close(reinterpret_cast<uv_handle_t*>(&client->handle), [](uv_handle_t* h) {
                delete static_cast<Client*>(h->data);
            });
            return 0;
        }
    }

    Value runnable(const std::vector<Value>& args) {
        if (args.size() < 4) {
            L_ERR("socket_create requires 4 arguments.");
        }

        std::string address = args[0].to_string();
        if (!args[1].is_numeric()) L_ERR("Second argument must be numeric (port)");
        int port = (int)args[1].as_number();

        if (!args[2].is_numeric()) L_ERR("Third argument must be numeric (protocol)");
        int protocol = (int)args[2].as_number();
        if (protocol != 4 && protocol != 6) {
            L_ERR("Protocol must be 4 (IPv4) or 6 (IPv6)");
        }

        if (!args[3].is_numeric()) L_ERR("Fourth argument must be numeric (type)");
        int type = (int)args[3].as_number();
        if (type != 0 && type != 1) {
            L_ERR("Type must be 0 (TCP) or 1 (UDP)");
        }

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
            L_ERR("socket_send requires 2 arguments.");
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

        it->second->send(data);
        return Value(0);
    }

    // 供脚本注册回调接口，传入回调id
    Value socket_register_receive_callback(const std::vector<Value>& args) {
        if (args.empty()) {
            L_ERR("socket_register_receive_callback requires 1 argument (callback_id)");
        }
        if (!args[0].is_numeric()) {
            L_ERR("callback_id must be numeric");
        }
        on_receive_callback_id = static_cast<int64_t>(args[0].as_number());
        std::cout << "Registered receive callback id = " << on_receive_callback_id << std::endl;
        return Value(static_cast<int>(on_receive_callback_id));
    }


    void call_lamina_callback(int64_t cbid, uint64_t client_id, const std::string& data) {

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
