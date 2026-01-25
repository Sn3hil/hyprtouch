#include "lockfile.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <signal.h>

#include "../ipc/socket.hpp"

namespace hyprtouch::daemon {

std::string LockFile::get_lock_path() {
    return ipc::Socket::get_runtime_dir() + "/hyprtouch.lock";
}

bool LockFile::is_daemon_running() {
    int pid = get_daemon_pid();
    if (pid <= 0) {
        return false;
    }
    
    // Check if process exists
    return kill(pid, 0) == 0;
}

int LockFile::get_daemon_pid() {
    std::ifstream file(get_lock_path());
    if (!file.is_open()) {
        return -1;
    }
    
    int pid;
    file >> pid;
    return pid;
}

LockFile::LockFile(const std::string& path) : path_(path) {}

LockFile::~LockFile() {
    release();
}

bool LockFile::acquire() {
    if (acquired_) {
        return true;
    }

    // Open/create lock file
    fd_ = open(path_.c_str(), O_RDWR | O_CREAT, 0600);
    if (fd_ == -1) {
        perror("Failed to open lock file");
        return false;
    }

    // Try to acquire exclusive lock
    if (flock(fd_, LOCK_EX | LOCK_NB) == -1) {
        if (errno == EWOULDBLOCK) {
            // Another instance is running
            close(fd_);
            fd_ = -1;
            return false;
        }
        perror("Failed to lock file");
        close(fd_);
        fd_ = -1;
        return false;
    }

    // Write our PID
    if (ftruncate(fd_, 0) == -1) {
        perror("Failed to truncate lock file");
        release();
        return false;
    }

    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    
    if (write(fd_, pid_str, strlen(pid_str)) == -1) {
        perror("Failed to write PID");
        release();
        return false;
    }

    acquired_ = true;
    return true;
}

void LockFile::release() {
    if (!acquired_) {
        return;
    }

    if (fd_ != -1) {
        flock(fd_, LOCK_UN);
        close(fd_);
        fd_ = -1;
    }

    // Remove lock file
    unlink(path_.c_str());
    acquired_ = false;
}

}  // namespace hyprtouch::daemon

