// Hyprland mouse control helpers
#pragma once

#include <functional>

namespace hyprtouch::mouse {

void set_pre_click_callback(std::function<void()> callback);
void set_post_click_quit(std::function<void()> callback);

bool move_cursor(int x, int y);

void click_left();
void click_right();
void click_middle();
void double_click_left();

}  

