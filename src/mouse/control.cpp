#include "control.hpp"

#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#include <sys/wait.h>

namespace hyprtouch::mouse {

namespace {

bool run_command(const std::string &cmd) {
    const int status = std::system(cmd.c_str());
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

std::function<void()> pre_click_callback;
std::function<void()> post_click_quit;

}  // namespace

void set_pre_click_callback(std::function<void()> callback) {
    pre_click_callback = std::move(callback);
}

void set_post_click_quit(std::function<void()> callback) {
    post_click_quit = std::move(callback);
}

bool move_cursor(int x, int y) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const std::string cmd = "hyprctl dispatch movecursor " + std::to_string(x) +
                            " " + std::to_string(y);
    return run_command(cmd);
}

void click_left() {
    if (pre_click_callback) {
        pre_click_callback();
    }
    auto callback = post_click_quit;
    std::thread([callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        run_command("wlrctl pointer click left");
        if (callback) {
            callback();
        }
    }).detach();
}

void click_right() {
    if (pre_click_callback) {
        pre_click_callback();
    }
    auto callback = post_click_quit;
    std::thread([callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        run_command("wlrctl pointer click right");
        if (callback) {
            callback();
        }
    }).detach();
}

void click_middle() {
    if (pre_click_callback) {
        pre_click_callback();
    }
    auto callback = post_click_quit;
    std::thread([callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        run_command("wlrctl pointer click middle");
        if (callback) {
            callback();
        }
    }).detach();
}

void double_click_left() {
    if (pre_click_callback) {
        pre_click_callback();
    }
    auto callback = post_click_quit;
    std::thread([callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        run_command("wlrctl pointer click left");
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        run_command("wlrctl pointer click left");
        if (callback) {
            callback();
        }
    }).detach();
}

}  
