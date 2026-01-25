#include "client.hpp"

#include <nlohmann/json.hpp>

namespace hyprtouch::ipc {

Client::Client() : socket_(std::make_unique<Socket>()) {}

std::optional<Response> Client::send_command(Command cmd, const std::string& socket_path) {
    socket_ = std::make_unique<Socket>();
    
    if (!socket_->connect(socket_path)) {
        return std::nullopt;
    }

    Request request{cmd};
    std::string request_str = request.to_json();

    if (!socket_->send(request_str)) {
        return std::nullopt;
    }

    std::string response_str = socket_->receive();
    if (response_str.empty()) {
        return std::nullopt;
    }

    try {
        auto j = nlohmann::json::parse(response_str);
        Response response;
        response.success = j["status"] == "ok";
        response.message = j.value("message", "");
        return response;
    } catch (...) {
        return std::nullopt;
    }
}

}  // namespace hyprtouch::ipc

