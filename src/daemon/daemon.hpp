// Main daemon class
#pragma once

#include <memory>
#include <gtk/gtk.h>

#include "../ipc/server.hpp"
#include "../overlay/window.hpp"
#include "../input/handler.hpp"
#include "lockfile.hpp"

namespace hyprtouch::daemon {

class Daemon {
public:
    Daemon();
    ~Daemon();

    bool start();
    void stop();
    
    int run(int argc, char** argv);

private:
    void setup_signal_handlers();
    static gboolean on_signal(gpointer user_data);
    
    ipc::Response handle_command(ipc::Command cmd);
    
    void show_overlay();
    void hide_overlay();
    void toggle_overlay();
    void reset_state();
    
    GtkApplication* app_{nullptr};
    std::unique_ptr<ipc::Server> ipc_server_;
    std::unique_ptr<LockFile> lock_file_;
    std::unique_ptr<overlay::OverlayWindow> window_;
    std::unique_ptr<input::Handler> handler_;
    
    bool overlay_visible_{false};
    bool running_{false};
};

}  // namespace hyprtouch::daemon

