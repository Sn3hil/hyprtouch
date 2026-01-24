#include "grid.hpp"

#include <pango/pangocairo.h>
#include <string>
#include <cmath>
namespace hyprtouch::overlay {

// Helper to get labels
static std::string get_row_char(int r) {
    const std::string rows = "dfghjk";
    if (r >= 0 && r < static_cast<int>(rows.size())) {
        return std::string(1, rows[r]);
    }
    return "?";
}

static std::string get_col_char(int c) {
    const std::string cols = "asdfghjkl;";
    if (c >= 0 && c < static_cast<int>(cols.size())) {
        return std::string(1, cols[c]);
    }
    return "?";
}

void draw_grid(cairo_t *cr, int width, int height,
               const mouse::Rect &base_area, const mouse::Rect &zoom_area,
               const GridConfig &config) {
    if (!cr || width <= 0 || height <= 0) {
        return;
    }

    const double scale_x = static_cast<double>(width) / base_area.width;
    const double scale_y = static_cast<double>(height) / base_area.height;

    const double grid_x = (zoom_area.x - base_area.x) * scale_x;
    const double grid_y = (zoom_area.y - base_area.y) * scale_y;
    const double grid_w = zoom_area.width * scale_x;
    const double grid_h = zoom_area.height * scale_y;

    // Draw semi-transparent overlay over entire screen
    // (R=0, G=0.2, B=0.4) with 25% opacity
    cairo_set_source_rgba(cr, 0.0, 0.2, 0.4, 0.25);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    const double cell_w = grid_w / config.cols;
    const double cell_h = grid_h / config.rows;

    // Draw row highlight if active
    if (config.highlight_row >= 0 && config.highlight_row < config.rows) {
        // If a column is also selected, highlight only the specific cell
        if (config.highlight_col >= 0 && config.highlight_col < config.cols) {
            const double cell_x = grid_x + config.highlight_col * cell_w;
            const double cell_y = grid_y + config.highlight_row * cell_h;
            cairo_set_source_rgba(cr, 0.2, 0.8, 0.2, 0.5); // Green-ish highlight for final selection
            cairo_rectangle(cr, cell_x, cell_y, cell_w, cell_h);
            cairo_fill(cr);
        } else {
            // highlight the entire row
            const double hy = grid_y + config.highlight_row * cell_h;
            cairo_set_source_rgba(cr, 0.4, 0.6, 1.0, 0.3); // Lighter blue highlight for row
            cairo_rectangle(cr, grid_x, hy, grid_w, cell_h);
            cairo_fill(cr);
        }
    }

    // Draw the grid within the zoom area
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    cairo_set_line_width(cr, 1.0);

    // Vertical lines.
    for (int c = 0; c <= config.cols; ++c) {
        const double x = grid_x + c * cell_w;
        cairo_move_to(cr, x, grid_y);
        cairo_line_to(cr, x, grid_y + grid_h);
    }

    // Horizontal lines.
    for (int r = 0; r <= config.rows; ++r) {
        const double y = grid_y + r * cell_h;
        cairo_move_to(cr, grid_x, y);
        cairo_line_to(cr, grid_x + grid_w, y);
    }
    cairo_stroke(cr);

    // Draw labels
    PangoLayout *layout = pango_cairo_create_layout(cr);

    // Scale font size based on cell size (smaller cells = smaller font).
    const int font_size = std::max(8, static_cast<int>(std::min(cell_w, cell_h) * 0.25));
    const std::string font_desc_str = "Sans " + std::to_string(font_size);
    PangoFontDescription *font_desc = pango_font_description_from_string(font_desc_str.c_str());
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);

    for (int r = 0; r < config.rows; ++r) {
        for (int c = 0; c < config.cols; ++c) {
            //label format: RowChar + ColChar (e.g., "da", "d;", "ka")
            const std::string label = get_row_char(r) + get_col_char(c);
            
            pango_layout_set_text(layout, label.c_str(), -1);

            int text_w = 0;
            int text_h = 0;
            pango_layout_get_pixel_size(layout, &text_w, &text_h);

            const double cx = grid_x + (c + 0.5) * cell_w - text_w / 2.0;
            const double cy = grid_y + (r + 0.5) * cell_h - text_h / 2.0;

            cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
            cairo_move_to(cr, cx, cy);
            pango_cairo_show_layout(cr, layout);
        }
    }

    g_object_unref(layout);
}

} 
