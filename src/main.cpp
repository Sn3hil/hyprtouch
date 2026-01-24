#include <gtk/gtk.h>

#include <memory>
#include <utility>

#include "hypr/monitor.hpp"
#include "input/handler.hpp"
#include "mouse/control.hpp"
#include "overlay/window.hpp"

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

}  

}  

int main(int argc, char **argv) {
    auto app = g_object_ref_sink(gtk_application_new("dev.hyprtouch.overlay",
                                                     G_APPLICATION_FLAGS_NONE));
    hyprtouch::AppState state;

    g_signal_connect(app, "activate", G_CALLBACK(hyprtouch::on_activate), &state);
    const int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Ensure objects are destroyed before unref.
    state.handler.reset();
    state.window.reset();
    g_object_unref(app);
    return status;
}

