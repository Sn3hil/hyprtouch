// Coordinate calculations for grid navigation
#pragma once

namespace hyprtouch::mouse {

struct Rect {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;

    double center_x() const { return x + width * 0.5; }
    double center_y() const { return y + height * 0.5; }
};

Rect subcell(const Rect &area, int row, int col, int rows, int cols);

}  

