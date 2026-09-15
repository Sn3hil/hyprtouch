// Overlay window that renders the grid and handles layout updates
#pragma once

#include <gtk/gtk.h>

#include "../hypr/monitor.hpp"
#include "../mouse/coords.hpp"
#include "grid.hpp"

namespace hyprtouch::overlay {

class OverlayWindow {
public:
    OverlayWindow(GtkApplication *app, const hypr::MonitorInfo &monitor);
    ~OverlayWindow();

    GtkWindow *window() const { return GTK_WINDOW(window_); }

    void reset_area();
    void set_monitor(const hypr::MonitorInfo &monitor);
    void set_area(const mouse::Rect &area);
    const mouse::Rect &area() const { return area_; }
    const mouse::Rect &base_area() const { return base_area_; }

    void set_highlight_row(int row);
    void set_highlight_cell(int row, int col);
    const GridConfig &grid_config() const { return grid_config_; }
    void queue_render();

private:
    void build_ui(GtkApplication *app);
    void update_layer_surface();

    static void draw_cb(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);

    GtkWidget *window_{nullptr};
    GtkWidget *drawing_area_{nullptr};

    hypr::MonitorInfo monitor_;
    mouse::Rect base_area_;
    mouse::Rect area_;
    GridConfig grid_config_{};
};

} 

