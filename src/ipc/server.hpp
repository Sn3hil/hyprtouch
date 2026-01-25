// IPC server for daemon
#pragma once

#include <functional>
#include <memory>
#include <gio/gio.h>

#include "socket.hpp"
#include "protocol.hpp"

namespace hyprtouch::ipc {

using CommandHandler = std::function<Response(Command)>;

class Server {
public:
    Server();
    ~Server();

    bool start(const std::string& socket_path);
    void stop();
    
    void set_command_handler(CommandHandler handler);
    
    bool is_running() const { return running_; }

private:
    static gboolean on_accept(GSocket* socket, GIOCondition condition, gpointer user_data);
    void handle_connection(GSocket* client_socket);
    
    std::unique_ptr<Socket> socket_;
    GSource* source_{nullptr};
    CommandHandler command_handler_;
    bool running_{false};
};

}  // namespace hyprtouch::ipc

