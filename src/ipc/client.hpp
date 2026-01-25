// IPC client for CLI
#pragma once

#include <string>
#include <optional>
#include <memory>

#include "socket.hpp"
#include "protocol.hpp"

namespace hyprtouch::ipc {

class Client {
public:
    Client();
    ~Client() = default;

    std::optional<Response> send_command(Command cmd, const std::string& socket_path);

private:
    std::unique_ptr<Socket> socket_;
};

}  // namespace hyprtouch::ipc

