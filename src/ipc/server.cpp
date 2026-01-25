#include "server.hpp"

#include <gio/gio.h>

namespace hyprtouch::ipc {

Server::Server() : socket_(std::make_unique<Socket>()) {}

Server::~Server() {
    stop();
}

void Server::set_command_handler(CommandHandler handler) {
    command_handler_ = std::move(handler);
}

bool Server::start(const std::string& socket_path) {
    if (running_) {
        return false;
    }

    if (!socket_->bind(socket_path)) {
        return false;
    }

    if (!socket_->listen()) {
        return false;
    }

    // Create a GSource for accepting connections
    source_ = g_socket_create_source(socket_->gsocket(), G_IO_IN, nullptr);
    g_source_set_callback(source_, (GSourceFunc)on_accept, this, nullptr);
    g_source_attach(source_, nullptr);

    running_ = true;
    return true;
}

void Server::stop() {
    if (!running_) {
        return;
    }

    if (source_) {
        g_source_destroy(source_);
        g_source_unref(source_);
        source_ = nullptr;
    }

    socket_->close();
    running_ = false;
}

gboolean Server::on_accept(GSocket* socket, GIOCondition condition, gpointer user_data) {
    auto* server = static_cast<Server*>(user_data);
    if (!server) {
        return G_SOURCE_REMOVE;
    }

    GError* error = nullptr;
    GSocket* client_socket = g_socket_accept(socket, nullptr, &error);

    if (error) {
        g_printerr("Failed to accept connection: %s\n", error->message);
        g_error_free(error);
        return G_SOURCE_CONTINUE;
    }

    if (client_socket) {
        server->handle_connection(client_socket);
        g_object_unref(client_socket);
    }

    return G_SOURCE_CONTINUE;
}

void Server::handle_connection(GSocket* client_socket) {
    char buffer[4096];
    GError* error = nullptr;

    gssize received = g_socket_receive(client_socket, buffer, sizeof(buffer) - 1, nullptr, &error);

    if (error) {
        g_printerr("Failed to receive from client: %s\n", error->message);
        g_error_free(error);
        return;
    }

    if (received <= 0) {
        return;
    }

    buffer[received] = '\0';
    std::string request_str(buffer, received);

    // Parse request
    auto request = Request::from_json(request_str);
    Response response = Response::error("Invalid request");

    if (request && command_handler_) {
        response = command_handler_(request->cmd);
    }

    // Send response
    std::string response_str = response.to_json();
    g_socket_send(client_socket, response_str.c_str(), response_str.size(), nullptr, nullptr);
}

}  // namespace hyprtouch::ipc

