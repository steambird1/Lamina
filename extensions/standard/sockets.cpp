#include "sockets.hpp"

int create_socket(uv_stream_t *server, int status) {
#ifdef __linux__
     if (status < 0) {
          return -1;
     }

     uv_tcp_t *client = new uv_tcp_t;
     uv_tcp_init(loop, client);
     if (uv_accept(server, (uv_stream_t*) client) == 0) {
          return 1;
     } else {
          uv_close((uv_handle_t*) client, [](uv_handle_t* handle) {
           delete (uv_tcp_t*) handle;
       });
     }
#endif
}

Value runnable(const std::vector<Value> &args) {
     std::string address = args[0].to_string();
     if (!args[1].is_numeric()) {
          L_ERR("Second Arg Must Be Number!");
     }
     int port = args[1].as_number();
     // protocol(IPV4/IPV6)
     if (!args[2].is_numeric()) {
          L_ERR("Third Arg Must Be Number!");
     }
     int protocol = args[2].as_number();
     if (protocol != 4 && protocol != 6) {
          L_ERR("Third Arg Must Be 4 Or 6!");
     }
     // TCP/UDP
     if (!args[3].is_numeric()) {
          L_ERR("Fourth Arg Must Be Number!");
     }
     int type = args[3].as_number();
     if (type != 0 && type != 1) {
          L_ERR("Fourth Arg Must Be 0 Or 1!");
     }
#ifdef __linux__
     if (protocol == 4 && type == 0) {
          loop = uv_default_loop();
          uv_tcp_init(loop, &server);

          uv_tcp_nodelay(&server, 1);

          struct sockaddr_in addr;
          uv_ip4_addr(address.c_str(), port, &addr);
          uv_tcp_bind(&server, (const sockaddr*)&addr, 0);

          int r = uv_listen((uv_stream_t*) &server, DEFAULT_BACKLOG,
              [](uv_stream_t* server, int status) {
                  create_socket(server, status);
              });

          if (r) {
               L_ERR("Listen error: " + std::string(uv_strerror(r)));
          }

          uv_run(loop, UV_RUN_DEFAULT);
          return Value(0);
     }
#endif
}
