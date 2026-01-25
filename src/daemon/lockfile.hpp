// Lock file management for daemon single-instance control
#pragma once

#include <string>

namespace hyprtouch::daemon {

class LockFile {
public:
    LockFile(const std::string& path);
    ~LockFile();

    bool acquire();
    void release();
    
    static std::string get_lock_path();
    static bool is_daemon_running();
    static int get_daemon_pid();

private:
    std::string path_;
    int fd_{-1};
    bool acquired_{false};
};

}  // namespace hyprtouch::daemon

