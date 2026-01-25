// Unix socket wrapper for IPC
#pragma once

#include <string>
#include <functional>
#include <gio/gio.h>

namespace hyprtouch::ipc {

class Socket {
public:
    Socket();
    ~Socket();

    // Server side
    bool bind(const std::string& path);
    bool listen();
    
    // Client side
    bool connect(const std::string& path);
    
    // I/O
    bool send(const std::string& data);
    std::string receive();
    
    // GLib integration for server
    GSocket* gsocket() const { return socket_; }
    
    void close();
    bool is_open() const { return socket_ != nullptr; }
    
    static std::string get_socket_path();
    static std::string get_runtime_dir();

private:
    GSocket* socket_{nullptr};
    GSocketAddress* address_{nullptr};
};

}  // namespace hyprtouch::ipc

