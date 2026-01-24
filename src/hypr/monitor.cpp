#include "monitor.hpp"

#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace hyprtouch::hypr {

namespace {

std::string run_command(const std::string &cmd) {
    std::array<char, 256> buffer{};
    std::string result;

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return result;
    }
    while (fgets(buffer.data(), buffer.size(), pipe)) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

} 

MonitorInfo focused_monitor() {
    MonitorInfo info;
    const std::string output = run_command("hyprctl monitors -j");
    if (output.empty()) {
        return info;
    }

    try {
        auto json = nlohmann::json::parse(output);
        if (!json.is_array()) {
            return info;
        }

        for (const auto &mon : json) {
            if (!mon.contains("focused") || !mon["focused"].is_boolean()) {
                continue;
            }
            if (!mon["focused"].get<bool>()) {
                continue;
            }

            info.x = mon.value("x", 0);
            info.y = mon.value("y", 0);
            info.width = mon.value("width", 0);
            info.height = mon.value("height", 0);
            info.scale = mon.value("scale", 1.0);
            break;
        }
    } catch (const std::exception &) {
        // parsing failed, ummmm...
    }

    return info;
}

} 

