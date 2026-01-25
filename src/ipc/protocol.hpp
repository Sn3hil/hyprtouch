// IPC protocol definitions and message serialization
#pragma once

#include <string>
#include <optional>

namespace hyprtouch::ipc {

enum class Command {
    TOGGLE,
    SHOW,
    HIDE,
    STATUS,
    QUIT
};

struct Request {
    Command cmd;
    
    static std::optional<Request> from_json(const std::string& json_str);
    std::string to_json() const;
};

struct Response {
    bool success;
    std::string message;
    
    std::string to_json() const;
    static Response ok(const std::string& msg = "OK");
    static Response error(const std::string& msg);
};

}  // namespace hyprtouch::ipc

