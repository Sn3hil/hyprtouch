// Cairo grid rendering helpers
#pragma once

#include <cairo.h>

#include "../mouse/coords.hpp"

namespace hyprtouch::overlay {

struct GridConfig {
    int rows = 6;
    int cols = 10;
    double overlay_opacity = 0.5;
    int highlight_row = -1;
    int highlight_col = -1;
};

void draw_grid(cairo_t *cr, int width, int height,
               const mouse::Rect &base_area, const mouse::Rect &zoom_area,
               const GridConfig &config);

} 
