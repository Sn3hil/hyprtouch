#include <gtk/gtk.h>
#include <getopt.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

#include <memory>
#include <utility>
#include <iostream>

#include "hypr/monitor.hpp"
#include "input/handler.hpp"
#include "mouse/control.hpp"
#include "overlay/window.hpp"
#include "daemon/daemon.hpp"
#include "daemon/lockfile.hpp"
#include "ipc/client.hpp"
#include "ipc/protocol.hpp"

namespace hyprtouch {

namespace {

struct AppState {
    std::unique_ptr<overlay::OverlayWindow> window;
    std::unique_ptr<input::Handler> handler;
};

gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint /*keycode*/,
                        GdkModifierType state, gpointer user_data) {
    auto *handler = static_cast<input::Handler *>(user_data);
    if (!handler) {
        return GDK_EVENT_PROPAGATE;
    }
    const bool handled = handler->handle_key_press(keyval, state);
    return handled ? GDK_EVENT_STOP : GDK_EVENT_PROPAGATE;
}

gboolean on_key_released(GtkEventControllerKey *controller, guint keyval, guint /*keycode*/,
                        GdkModifierType state, gpointer user_data) {
    auto *handler = static_cast<input::Handler *>(user_data);
    if (!handler) {
        return GDK_EVENT_PROPAGATE;
    }
    const bool handled = handler->handle_key_release(keyval, state);
    return handled ? GDK_EVENT_STOP : GDK_EVENT_PROPAGATE;
}

void on_activate(GtkApplication *app, gpointer user_data) {
    auto *state = static_cast<AppState *>(user_data);
    if (!state) {
        g_printerr("Internal error: missing app state\n");
        g_application_quit(G_APPLICATION(app));
        return;
    }

    const auto monitor = hypr::focused_monitor();
    if (!monitor.valid()) {
        g_printerr("Failed to detect focused monitor via hyprctl\n");
        g_application_quit(G_APPLICATION(app));
        return;
    }

    state->window = std::make_unique<overlay::OverlayWindow>(app, monitor);
    state->handler = std::make_unique<input::Handler>(*state->window, [app]() {
        g_application_quit(G_APPLICATION(app));
    });

    // Hide window before clicking (but don't destroy it yet)
    mouse::set_pre_click_callback([state]() {
        if (state->window) {
            gtk_widget_hide(GTK_WIDGET(state->window->window()));
        }
    });

    // Queue app quit after click completes (runs in main loop)
    mouse::set_post_click_quit([app]() {
        g_idle_add([](gpointer data) -> gboolean {
            g_application_quit(G_APPLICATION(data));
            return FALSE;
        }, app);
    });

    GtkWindow *gtk_window = state->window->window();
    GtkEventController *key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), state->handler.get());
    g_signal_connect(key_controller, "key-released", G_CALLBACK(on_key_released), state->handler.get());
    gtk_widget_add_controller(GTK_WIDGET(gtk_window), key_controller);

    gtk_widget_show(GTK_WIDGET(gtk_window));
    gtk_window_present(gtk_window);
}

int run_legacy_mode(int argc, char **argv) {
    auto app = g_object_ref_sink(gtk_application_new("dev.hyprtouch.overlay",
                                                     G_APPLICATION_FLAGS_NONE));
    AppState state;

    g_signal_connect(app, "activate", G_CALLBACK(on_activate), &state);
    // Don't pass args to GTK to avoid parsing errors
    const int status = g_application_run(G_APPLICATION(app), 0, nullptr);

    // Ensure objects are destroyed before unref.
    state.handler.reset();
    state.window.reset();
    g_object_unref(app);
    return status;
}

bool wait_for_socket(const std::string& socket_path, int timeout_ms) {
    const int interval_ms = 50;
    int elapsed_ms = 0;
    
    while (elapsed_ms < timeout_ms) {
        if (access(socket_path.c_str(), F_OK) == 0) {
            // Socket exists, wait a bit more for it to be ready
            usleep(100000); // 100ms
            return true;
        }
        usleep(interval_ms * 1000);
        elapsed_ms += interval_ms;
    }
    
    return false;
}

bool ensure_daemon_running() {
    if (daemon::LockFile::is_daemon_running()) {
        return true;
    }
    
    // Fork and start daemon
    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork");
        return false;
    }
    
    if (pid == 0) {
        // Child process: become daemon
        setsid();
        
        // Execute daemon
        execl("/proc/self/exe", "hyprtouch", "--daemon", nullptr);
        
        // If exec fails
        perror("Failed to exec daemon");
        _exit(1);
    }
    
    // Parent: wait for socket to be ready
    std::string socket_path = ipc::Socket::get_socket_path();
    if (!wait_for_socket(socket_path, 2000)) {
        g_printerr("Timeout waiting for daemon to start\n");
        return false;
    }
    
    return true;
}

int send_ipc_command(ipc::Command cmd) {
    if (!ensure_daemon_running()) {
        return 1;
    }
    
    ipc::Client client;
    auto response = client.send_command(cmd, ipc::Socket::get_socket_path());
    
    if (!response) {
        g_printerr("Failed to communicate with daemon\n");
        return 1;
    }
    
    if (!response->message.empty()) {
        std::cout << response->message << std::endl;
    }
    
    return response->success ? 0 : 1;
}

void print_help(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n\n"
              << "Options:\n"
              << "  --daemon           Start daemon mode\n"
              << "  --toggle-overlay   Toggle overlay visibility (default)\n"
              << "  --show             Show overlay\n"
              << "  --hide             Hide overlay\n"
              << "  --status           Check daemon status\n"
              << "  --stop             Stop daemon\n"
              << "  --legacy           Run in legacy single-run mode\n"
              << "  --help             Show this help message\n";
}

}  // namespace

}  // namespace hyprtouch

int main(int argc, char **argv) {
    static struct option long_options[] = {
        {"daemon", no_argument, 0, 'd'},
        {"toggle-overlay", no_argument, 0, 't'},
        {"show", no_argument, 0, 's'},
        {"hide", no_argument, 0, 'H'},
        {"status", no_argument, 0, 'S'},
        {"stop", no_argument, 0, 'q'},
        {"legacy", no_argument, 0, 'l'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    bool action_taken = false;
    
    // Loop to process all options, though usually only one is provided
    while ((c = getopt_long(argc, argv, "dtsHSql?", long_options, &option_index)) != -1) {
        action_taken = true;
        switch (c) {
            case 'd':
                // Run daemon
                {
                    hyprtouch::daemon::Daemon daemon;
                    return daemon.run(argc, argv);
                }
                
            case 't':
                return hyprtouch::send_ipc_command(hyprtouch::ipc::Command::TOGGLE);
                
            case 's':
                return hyprtouch::send_ipc_command(hyprtouch::ipc::Command::SHOW);
                
            case 'H':
                return hyprtouch::send_ipc_command(hyprtouch::ipc::Command::HIDE);
                
            case 'S':
                // Check status explicitly without auto-starting
                if (hyprtouch::daemon::LockFile::is_daemon_running()) {
                    std::cout << "Daemon is running" << std::endl;
                    return 0;
                } else {
                    std::cout << "Daemon is not running" << std::endl;
                    return 1;
                }
                
            case 'q':
                // Stop daemon explicitly without auto-starting
                if (hyprtouch::daemon::LockFile::is_daemon_running()) {
                     hyprtouch::ipc::Client client;
                     auto response = client.send_command(hyprtouch::ipc::Command::QUIT, 
                                                         hyprtouch::ipc::Socket::get_socket_path());
                     if (response && !response->message.empty()) {
                         std::cout << response->message << std::endl;
                     }
                     return 0;
                } else {
                    std::cout << "Daemon is not running" << std::endl;
                    return 1;
                }
                
            case 'l':
                return hyprtouch::run_legacy_mode(argc, argv);
                
            case '?':
            default:
                hyprtouch::print_help(argv[0]);
                return (c == '?') ? 0 : 1;
        }
    }
    
    // if no arguments
    if (!action_taken) {
         std::cout << "Unknown Option" << std::endl;
         std::cout << "use hyprtouch --help" << std::endl;

    }
    
    return 0;
}
