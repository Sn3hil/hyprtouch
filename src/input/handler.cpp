#include "handler.hpp"

#include <gtk/gtk.h>

#include "../mouse/control.hpp"
#include "../mouse/coords.hpp"

namespace hyprtouch::input {

Handler::Handler(overlay::OverlayWindow &window, std::function<void()> on_close)
    : window_(window), on_close_(std::move(on_close)) {}

int Handler::grid_value_from_key(guint keyval, bool is_row) const {
    if (is_row) {
        // Map d-k to 0-5
        switch (keyval) {
            case GDK_KEY_d: return 0;
            case GDK_KEY_f: return 1;
            case GDK_KEY_g: return 2;
            case GDK_KEY_h: return 3;
            case GDK_KEY_j: return 4;
            case GDK_KEY_k: return 5;
            default: return -1;
        }
    } else {
        // Map a-; to 0-9
        switch (keyval) {
            case GDK_KEY_a: return 0;
            case GDK_KEY_s: return 1;
            case GDK_KEY_d: return 2;
            case GDK_KEY_f: return 3;
            case GDK_KEY_g: return 4;
            case GDK_KEY_h: return 5;
            case GDK_KEY_j: return 6;
            case GDK_KEY_k: return 7;
            case GDK_KEY_l: return 8;
            case GDK_KEY_semicolon: return 9;
            default: return -1;
        }
    }
}

bool Handler::handle_key_press(guint keyval, GdkModifierType state) {
    // navigation (Shift + vim/WASD, or arrows)
    bool is_nav = false;
    int d_row = 0;
    int d_col = 0;

    // check arrow keys
    if (keyval == GDK_KEY_Left || keyval == GDK_KEY_Right || 
        keyval == GDK_KEY_Up || keyval == GDK_KEY_Down) {
        is_nav = true;
        switch (keyval) {
            case GDK_KEY_Left:  d_col = -1; break;
            case GDK_KEY_Right: d_col =  1; break;
            case GDK_KEY_Up:    d_row = -1; break;
            case GDK_KEY_Down:  d_row =  1; break;
        }
    } 
    // Check for Shift + [Vim/WASD]
    else if (state & GDK_SHIFT_MASK) {
        switch (keyval) {
            case GDK_KEY_h: case GDK_KEY_H: case GDK_KEY_a: case GDK_KEY_A: 
                d_col = -1; is_nav = true; break;
            case GDK_KEY_l: case GDK_KEY_L: case GDK_KEY_d: case GDK_KEY_D: 
                d_col =  1; is_nav = true; break;
            case GDK_KEY_j: case GDK_KEY_J: case GDK_KEY_s: case GDK_KEY_S: 
                d_row =  1; is_nav = true; break;
            case GDK_KEY_k: case GDK_KEY_K: case GDK_KEY_w: case GDK_KEY_W: 
                d_row = -1; is_nav = true; break;
        }
    }

    if (is_nav) {
        waiting_for_click_ = false;
        move_selection(d_row, d_col);
        return true;
    }

    // confirmation (Space/Enter)
    if (keyval == GDK_KEY_space || keyval == GDK_KEY_Return) {
        if (waiting_for_click_) {
            pending_click_ = true;
        } else if (selected_row_ != -1 && selected_col_ != -1) {
            // coordinates selected
            confirm_selection();
            if (waiting_for_click_) {
                pending_click_ = true;
            }
        } else {
            pending_click_ = true;
        }
        return true;
    }

    if (keyval == GDK_KEY_Escape) {
        if (waiting_for_click_ || !current_path_.empty()) {
            waiting_for_click_ = false;
            pending_click_ = false;
            current_path_.clear();
            selected_row_ = -1;
            selected_col_ = -1;
            window_.set_highlight_row(-1);
            return true;
        }
        if (on_close_) on_close_();
        return true;
    }

    // determine if looking for Row (0 items) or Col (1 item)
    bool is_row_input = current_path_.empty();
    int val = grid_value_from_key(keyval, is_row_input);

    if (val != -1) {
        waiting_for_click_ = false;
        pending_click_ = false;
        
        current_path_.push_back(val);
        
        if (current_path_.size() == 1) {
            // Row selected
            int row_idx = current_path_[0];
            window_.set_highlight_row(row_idx);
            selected_row_ = row_idx; 
            // selected_col_ remains -1
        } else if (current_path_.size() == 2) {
            process_selection();
        }
        return true;
    }

    return false;
}

bool Handler::handle_key_release(guint keyval, GdkModifierType state) {
    if ((keyval == GDK_KEY_space || keyval == GDK_KEY_Return) && pending_click_) {
        // Execute the deferred click action
        if (state & GDK_SHIFT_MASK) {
            mouse::double_click_left();
        } else if (state & GDK_ALT_MASK) {
            mouse::click_right();
        } else if (state & GDK_CONTROL_MASK) {
            mouse::click_middle();
        } else {
            mouse::click_left();
        }
        
        pending_click_ = false;
        waiting_for_click_ = false; // Done
        // on_close_() is handled by post_click_quit callback
        return true;
    }
    return false;
}

void Handler::process_selection() {
    if (current_path_.size() != 2) {
        current_path_.clear();
        window_.set_highlight_row(-1);
        return;
    }

    int row_idx = current_path_[0];
    int col_idx = current_path_[1];
    
    current_path_.clear();

    if (row_idx < 0 || row_idx >= kRows || col_idx < 0 || col_idx >= kCols) {
        window_.set_highlight_row(-1);
        return;
    }

    mouse::Rect target = mouse::subcell(window_.area(), row_idx, col_idx, kRows, kCols);

    // Always move cursor to the center of the selection
    mouse::move_cursor(static_cast<int>(target.center_x()), static_cast<int>(target.center_y()));

    if (level_ < kMaxLevel) {
        // Zoom in to the next level
        ++level_;
        window_.set_area(target);
        window_.set_highlight_row(-1); 
        selected_row_ = -1;
        selected_col_ = -1;
        return;
    }

    // Final level
    window_.set_highlight_cell(row_idx, col_idx);
    selected_row_ = row_idx;
    selected_col_ = col_idx;
    
    waiting_for_click_ = true;
}

void Handler::move_selection(int d_row, int d_col) {
    if (selected_row_ == -1 || selected_col_ == -1) {
        selected_row_ = 0;
        selected_col_ = 0;
    } else {
        selected_row_ += d_row;
        selected_col_ += d_col;
    }

    if (selected_row_ < 0) selected_row_ = 0;
    if (selected_row_ >= kRows) selected_row_ = kRows - 1;
    
    if (selected_col_ < 0) selected_col_ = 0;
    if (selected_col_ >= kCols) selected_col_ = kCols - 1;

    // Clear grid input
    current_path_.clear();

    window_.set_highlight_cell(selected_row_, selected_col_);
}

void Handler::confirm_selection() {
    if (selected_row_ == -1 || selected_col_ == -1) {
        return;
    }

    mouse::Rect target = mouse::subcell(window_.area(), selected_row_, selected_col_, kRows, kCols);

    mouse::move_cursor(static_cast<int>(target.center_x()), static_cast<int>(target.center_y()));

    if (level_ < kMaxLevel) {
        ++level_;
        window_.set_area(target);
        selected_row_ = -1;
        selected_col_ = -1;
        window_.set_highlight_row(-1);
    } else {
        waiting_for_click_ = true;        
    }
}

} 
