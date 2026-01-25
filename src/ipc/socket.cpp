#include "socket.hpp"

#include <gio/gunixsocketaddress.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

namespace hyprtouch::ipc {

std::string Socket::get_runtime_dir() {
    const char* runtime_dir = std::getenv("XDG_RUNTIME_DIR");
    if (runtime_dir && runtime_dir[0]) {
        return runtime_dir;
    }
    return "/tmp";
}

std::string Socket::get_socket_path() {
    return get_runtime_dir() + "/hyprtouch.sock";
}

Socket::Socket() {
    GError* error = nullptr;
    socket_ = g_socket_new(G_SOCKET_FAMILY_UNIX,
                           G_SOCKET_TYPE_STREAM,
                           G_SOCKET_PROTOCOL_DEFAULT,
                           &error);
    if (error) {
        g_printerr("Failed to create socket: %s\n", error->message);
        g_error_free(error);
        socket_ = nullptr;
    }
}

Socket::~Socket() {
    close();
}

void Socket::close() {
    if (socket_) {
        g_socket_close(socket_, nullptr);
        g_object_unref(socket_);
        socket_ = nullptr;
    }
    if (address_) {
        g_object_unref(address_);
        address_ = nullptr;
    }
}

bool Socket::bind(const std::string& path) {
    if (!socket_) {
        return false;
    }

    // Remove existing socket file
    ::unlink(path.c_str());

    address_ = g_unix_socket_address_new(path.c_str());
    
    GError* error = nullptr;
    gboolean result = g_socket_bind(socket_, address_, TRUE, &error);
    
    if (error) {
        g_printerr("Failed to bind socket: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    
    return result;
}

bool Socket::listen() {
    if (!socket_) {
        return false;
    }
    
    GError* error = nullptr;
    gboolean result = g_socket_listen(socket_, &error);
    
    if (error) {
        g_printerr("Failed to listen on socket: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    
    return result;
}

bool Socket::connect(const std::string& path) {
    if (!socket_) {
        return false;
    }

    GSocketAddress* addr = g_unix_socket_address_new(path.c_str());
    
    GError* error = nullptr;
    gboolean result = g_socket_connect(socket_, addr, nullptr, &error);
    
    g_object_unref(addr);
    
    if (error) {
        g_printerr("Failed to connect to socket: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    
    return result;
}

bool Socket::send(const std::string& data) {
    if (!socket_) {
        return false;
    }
    
    GError* error = nullptr;
    gssize sent = g_socket_send(socket_, data.c_str(), data.size(), nullptr, &error);
    
    if (error) {
        g_printerr("Failed to send data: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    
    return sent == static_cast<gssize>(data.size());
}

std::string Socket::receive() {
    if (!socket_) {
        return "";
    }
    
    char buffer[4096];
    GError* error = nullptr;
    
    gssize received = g_socket_receive(socket_, buffer, sizeof(buffer) - 1, nullptr, &error);
    
    if (error) {
        g_printerr("Failed to receive data: %s\n", error->message);
        g_error_free(error);
        return "";
    }
    
    if (received <= 0) {
        return "";
    }
    
    buffer[received] = '\0';
    return std::string(buffer, received);
}

}  // namespace hyprtouch::ipc

