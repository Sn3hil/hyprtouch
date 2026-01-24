#include "coords.hpp"

namespace hyprtouch::mouse {

Rect subcell(const Rect &area, int row, int col, int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        return area;
    }
    const double cell_w = area.width / static_cast<double>(cols);
    const double cell_h = area.height / static_cast<double>(rows);

    Rect cell;
    cell.x = area.x + col * cell_w;
    cell.y = area.y + row * cell_h;
    cell.width = cell_w;
    cell.height = cell_h;
    return cell;
}

}  

