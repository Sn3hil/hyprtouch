#include "protocol.hpp"

#include <nlohmann/json.hpp>

namespace hyprtouch::ipc {

std::optional<Request> Request::from_json(const std::string& json_str) {
    try {
        auto j = nlohmann::json::parse(json_str);
        if (!j.contains("cmd") || !j["cmd"].is_string()) {
            return std::nullopt;
        }
        
        std::string cmd_str = j["cmd"];
        Request req;
        
        if (cmd_str == "toggle") {
            req.cmd = Command::TOGGLE;
        } else if (cmd_str == "show") {
            req.cmd = Command::SHOW;
        } else if (cmd_str == "hide") {
            req.cmd = Command::HIDE;
        } else if (cmd_str == "status") {
            req.cmd = Command::STATUS;
        } else if (cmd_str == "quit") {
            req.cmd = Command::QUIT;
        } else {
            return std::nullopt;
        }
        
        return req;
    } catch (...) {
        return std::nullopt;
    }
}

std::string Request::to_json() const {
    nlohmann::json j;
    
    switch (cmd) {
        case Command::TOGGLE:
            j["cmd"] = "toggle";
            break;
        case Command::SHOW:
            j["cmd"] = "show";
            break;
        case Command::HIDE:
            j["cmd"] = "hide";
            break;
        case Command::STATUS:
            j["cmd"] = "status";
            break;
        case Command::QUIT:
            j["cmd"] = "quit";
            break;
    }
    
    return j.dump() + "\n";
}

Response Response::ok(const std::string& msg) {
    return Response{true, msg};
}

Response Response::error(const std::string& msg) {
    return Response{false, msg};
}

std::string Response::to_json() const {
    nlohmann::json j;
    j["status"] = success ? "ok" : "error";
    j["message"] = message;
    return j.dump() + "\n";
}

}  // namespace hyprtouch::ipc

