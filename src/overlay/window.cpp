#include "window.hpp"

#include <gtk4-layer-shell/gtk4-layer-shell.h>

namespace hyprtouch::overlay {

namespace {

GdkMonitor *find_monitor(const hypr::MonitorInfo &info) {
    GdkDisplay *display = gdk_display_get_default();
    if (!display) {
        return nullptr;
    }

    if (GListModel *monitors = gdk_display_get_monitors(display)) {
        const guint count = g_list_model_get_n_items(monitors);
        for (guint i = 0; i < count; ++i) {
            auto *mon = static_cast<GdkMonitor *>(g_list_model_get_item(monitors, i));
            if (!mon) {
                continue;
            }
            GdkRectangle rect{};
            gdk_monitor_get_geometry(mon, &rect);
            if (rect.x == info.x && rect.y == info.y &&
                rect.width == info.width && rect.height == info.height) {
                return mon;
            }
        }
        if (count > 0) {
            return static_cast<GdkMonitor *>(g_list_model_get_item(monitors, 0));
        }
    }

    return nullptr;
}

}  

OverlayWindow::OverlayWindow(GtkApplication *app, const hypr::MonitorInfo &monitor)
    : monitor_(monitor) {
    base_area_.x = monitor.x;
    base_area_.y = monitor.y;
    base_area_.width = monitor.width;
    base_area_.height = monitor.height;
    area_ = base_area_;

    build_ui(app);
}

OverlayWindow::~OverlayWindow() {
    if (window_) {
        gtk_window_destroy(GTK_WINDOW(window_));
        window_ = nullptr;
    }
}

void OverlayWindow::build_ui(GtkApplication *app) {
    window_ = gtk_application_window_new(app);
    gtk_window_set_decorated(GTK_WINDOW(window_), false);
    gtk_window_set_resizable(GTK_WINDOW(window_), false);
    gtk_window_set_title(GTK_WINDOW(window_), "hyprtouch Overlay");

    // Layer-shell initialization
    gtk_layer_init_for_window(GTK_WINDOW(window_));
    if (GdkMonitor *mon = find_monitor(monitor_)) {
        gtk_layer_set_monitor(GTK_WINDOW(window_), mon);
    }
    gtk_layer_set_layer(GTK_WINDOW(window_), GTK_LAYER_SHELL_LAYER_OVERLAY);
    
    // exclusive_zone = false
    gtk_layer_set_exclusive_zone(GTK_WINDOW(window_), -1); 
    
    gtk_layer_set_keyboard_mode(GTK_WINDOW(window_), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
    gtk_layer_set_namespace(GTK_WINDOW(window_), "hyprtouch");
    gtk_layer_set_anchor(GTK_WINDOW(window_), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window_), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window_), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window_), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_margin(GTK_WINDOW(window_), GTK_LAYER_SHELL_EDGE_TOP, 0);
    gtk_layer_set_margin(GTK_WINDOW(window_), GTK_LAYER_SHELL_EDGE_LEFT, 0);
    gtk_layer_set_margin(GTK_WINDOW(window_), GTK_LAYER_SHELL_EDGE_RIGHT, 0);
    gtk_layer_set_margin(GTK_WINDOW(window_), GTK_LAYER_SHELL_EDGE_BOTTOM, 0);

    // Drawing area
    drawing_area_ = gtk_drawing_area_new();
    gtk_widget_set_hexpand(drawing_area_, TRUE);
    gtk_widget_set_vexpand(drawing_area_, TRUE);
    gtk_widget_set_halign(drawing_area_, GTK_ALIGN_FILL);
    gtk_widget_set_valign(drawing_area_, GTK_ALIGN_FILL);
    gtk_widget_set_size_request(drawing_area_, monitor_.width, monitor_.height);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area_), draw_cb, this, nullptr);

    gtk_window_set_child(GTK_WINDOW(window_), drawing_area_);

    // Make the window background transparent so Cairo can handle the alpha blending
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "window { background-color: transparent; }", -1);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

void OverlayWindow::reset_area() {
    area_ = base_area_;
    queue_render();
}

void OverlayWindow::set_area(const mouse::Rect &area) {
    area_ = area;
    queue_render();
}

void OverlayWindow::set_highlight_row(int row) {
    grid_config_.highlight_row = row;
    grid_config_.highlight_col = -1; // Reset col when setting row
    queue_render();
}

void OverlayWindow::set_highlight_cell(int row, int col) {
    grid_config_.highlight_row = row;
    grid_config_.highlight_col = col;
    queue_render();
}

void OverlayWindow::queue_render() {
    if (drawing_area_) {
        gtk_widget_queue_draw(drawing_area_);
    }
}

void OverlayWindow::draw_cb(GtkDrawingArea * /*area*/, cairo_t *cr, int width, int height, gpointer user_data) {
    auto *self = static_cast<OverlayWindow *>(user_data);
    if (!self) {
        return;
    }
    draw_grid(cr, width, height, self->base_area_, self->area_, self->grid_config_);
}

}  
