#include "daemon.hpp"

#include <glib-unix.h>
#include "../hypr/monitor.hpp"
#include "../mouse/control.hpp"

namespace hyprtouch::daemon {

namespace {

struct DaemonActivateData {
    Daemon* daemon;
    GtkApplication* app;
};

void on_activate(GtkApplication* app, gpointer user_data) {
    // Hold the application to keep it running without windows
    g_application_hold(G_APPLICATION(app));
}

}  // namespace

Daemon::Daemon() {
    lock_file_ = std::make_unique<LockFile>(LockFile::get_lock_path());
    ipc_server_ = std::make_unique<ipc::Server>();
}

Daemon::~Daemon() {
    stop();
}

void Daemon::setup_signal_handlers() {
    g_unix_signal_add(SIGTERM, on_signal, this);
    g_unix_signal_add(SIGINT, on_signal, this);
}

gboolean Daemon::on_signal(gpointer user_data) {
    auto* daemon = static_cast<Daemon*>(user_data);
    if (daemon) {
        daemon->stop();
    }
    return G_SOURCE_REMOVE;
}

bool Daemon::start() {
    if (running_) {
        return false;
    }

    // Acquire lock file
    if (!lock_file_->acquire()) {
        g_printerr("Another instance of hyprtouch daemon is already running\n");
        return false;
    }

    // Start IPC server
    if (!ipc_server_->start(ipc::Socket::get_socket_path())) {
        g_printerr("Failed to start IPC server\n");
        lock_file_->release();
        return false;
    }

    // Set command handler
    ipc_server_->set_command_handler([this](ipc::Command cmd) {
        return handle_command(cmd);
    });

    running_ = true;
    return true;
}

void Daemon::stop() {
    if (!running_) {
        return;
    }

    // Hide overlay if visible
    if (overlay_visible_) {
        hide_overlay();
    }

    // Stop IPC server
    if (ipc_server_) {
        ipc_server_->stop();
    }

    // Release lock file
    if (lock_file_) {
        lock_file_->release();
    }

    // Quit GTK application
    if (app_) {
        g_application_quit(G_APPLICATION(app_));
    }

    running_ = false;
}

ipc::Response Daemon::handle_command(ipc::Command cmd) {
    switch (cmd) {
        case ipc::Command::TOGGLE:
            toggle_overlay();
            return ipc::Response::ok(overlay_visible_ ? "Overlay shown" : "Overlay hidden");
            
        case ipc::Command::SHOW:
            show_overlay();
            return ipc::Response::ok("Overlay shown");
            
        case ipc::Command::HIDE:
            hide_overlay();
            return ipc::Response::ok("Overlay hidden");
            
        case ipc::Command::STATUS:
            return ipc::Response::ok(running_ ? "Daemon running" : "Daemon not running");
            
        case ipc::Command::QUIT:
            // Schedule stop on main loop
            g_idle_add([](gpointer data) -> gboolean {
                auto* daemon = static_cast<Daemon*>(data);
                daemon->stop();
                return FALSE;
            }, this);
            return ipc::Response::ok("Daemon stopping");
            
        default:
            return ipc::Response::error("Unknown command");
    }
}

void Daemon::show_overlay() {
    if (overlay_visible_) {
        return;
    }

    const auto monitor = hypr::focused_monitor();
    if (!monitor.valid()) {
        g_printerr("Failed to detect focused monitor\n");
        return;
    }

    // Create or update window setup
    if (!window_) {
        window_ = std::make_unique<overlay::OverlayWindow>(app_, monitor);
        handler_ = std::make_unique<input::Handler>(*window_, [this]() {
            hide_overlay();
        });

        // Setup key event handlers
        GtkWindow* gtk_window = window_->window();
        GtkEventController* key_controller = gtk_event_controller_key_new();
        
        g_signal_connect(key_controller, "key-pressed", 
            G_CALLBACK(+[](GtkEventControllerKey* /*controller*/, guint keyval, 
                          guint /*keycode*/, GdkModifierType state, gpointer user_data) -> gboolean {
                auto* handler = static_cast<input::Handler*>(user_data);
                return handler->handle_key_press(keyval, state) ? GDK_EVENT_STOP : GDK_EVENT_PROPAGATE;
            }), handler_.get());
            
        g_signal_connect(key_controller, "key-released", 
            G_CALLBACK(+[](GtkEventControllerKey* /*controller*/, guint keyval, 
                          guint /*keycode*/, GdkModifierType state, gpointer user_data) -> gboolean {
                auto* handler = static_cast<input::Handler*>(user_data);
                return handler->handle_key_release(keyval, state) ? GDK_EVENT_STOP : GDK_EVENT_PROPAGATE;
            }), handler_.get());
            
        gtk_widget_add_controller(GTK_WIDGET(gtk_window), key_controller);

        // Setup mouse control callbacks
        mouse::set_pre_click_callback([this]() {
            if (window_) {
                gtk_widget_hide(GTK_WIDGET(window_->window()));
            }
        });

        mouse::set_post_click_hide([this]() {
            g_idle_add([](gpointer data) -> gboolean {
                auto* daemon = static_cast<Daemon*>(data);
                daemon->hide_overlay();
                return FALSE;
            }, this);
        });
    } else {
        window_->set_monitor(monitor);
    }

    // Reset state and show window
    reset_state();
    gtk_widget_show(GTK_WIDGET(window_->window()));
    gtk_window_present(window_->window());
    
    overlay_visible_ = true;
}

void Daemon::hide_overlay() {
    if (!overlay_visible_) {
        return;
    }

    if (window_) {
        gtk_widget_hide(GTK_WIDGET(window_->window()));
    }

    reset_state();
    overlay_visible_ = false;
}

void Daemon::toggle_overlay() {
    if (overlay_visible_) {
        hide_overlay();
    } else {
        show_overlay();
    }
}

void Daemon::reset_state() {
    if (window_) {
        window_->reset_area();
        window_->set_highlight_row(-1);
    }
    
    if (handler_) {
        handler_->reset();
    }
}

int Daemon::run(int argc, char** argv) {
    app_ = g_object_ref_sink(gtk_application_new("dev.hyprtouch.daemon",
                                                  G_APPLICATION_FLAGS_NONE));
    
    DaemonActivateData data{this, app_};
    g_signal_connect(app_, "activate", G_CALLBACK(on_activate), &data);

    setup_signal_handlers();

    if (!start()) {
        g_object_unref(app_);
        return 1;
    }

    const int status = g_application_run(G_APPLICATION(app_), 0, nullptr);

    // Cleanup
    handler_.reset();
    window_.reset();
    g_object_unref(app_);
    
    return status;
}

}  // namespace hyprtouch::daemon

