// Keyboard input handling 
#pragma once

#include <functional>
#include <string>

#include <gdk/gdk.h>

#include "../overlay/window.hpp"

namespace hyprtouch::input {

class Handler {
public:
    Handler(overlay::OverlayWindow &window, std::function<void()> on_close);

    // Returns true if the event was handled.
    bool handle_key_press(guint keyval, GdkModifierType state);
    bool handle_key_release(guint keyval, GdkModifierType state);
    
    void reset();

private:
    void process_selection();
    void move_selection(int d_row, int d_col);
    void confirm_selection();

    int grid_value_from_key(guint keyval, bool is_row) const;

    overlay::OverlayWindow &window_;
    std::function<void()> on_close_;

    // Stores 0-based indices. Size 0: nothing. Size 1: row selected. Size 2: full selection.
    std::vector<int> current_path_; 
    
    int selected_row_ = -1;
    int selected_col_ = -1;
    int level_ = 1;
    bool waiting_for_click_ = false;
    bool pending_click_ = false; // specific to Space/Enter handling
    static constexpr int kMaxLevel = 2;
    static constexpr int kRows = 6;
    static constexpr int kCols = 10;
};

}

