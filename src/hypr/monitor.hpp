// Monitor detection utilities for Hyprland
#pragma once

#include <string>

namespace hyprtouch::hypr {

struct MonitorInfo {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    double scale = 1.0;

    bool valid() const { return width > 0 && height > 0; }
};

MonitorInfo focused_monitor();

} 

