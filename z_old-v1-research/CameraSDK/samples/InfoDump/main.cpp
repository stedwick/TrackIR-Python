#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <atomic>
#include <csignal>
#include <cstdlib>

#include "KeyPress.h"
#include "DeviceConnectionManager.h"
#include "cameralibrary.h"

using namespace CameraLibrary;

// ---- Global stop flag ----------------------------------
static std::atomic_bool g_running{true};
static std::atomic_bool g_lib_shutdown_done{false};

static void on_signal(int) {
    g_running.store(false, std::memory_order_release);
}

// ---- Ensure CameraLibrary shuts down -------------------
struct ShutdownGuard {
    void finalize() {
        bool expected = false;
        if (!g_lib_shutdown_done.compare_exchange_strong(expected, true)) {
            return; // already shut down somewhere else
        }
        try {
            CameraManager::X().Shutdown();
        } catch (...) {
            // swallow on shutdown
        }
    }
    ~ShutdownGuard() { finalize(); }
};

struct TerminalModeGuard {
    explicit TerminalModeGuard(bool enable_now) : enabled(enable_now) {
        if (enabled) setTerminalMode(true);
    }
    ~TerminalModeGuard() {
        if (enabled) setTerminalMode(false);
    }
    bool enabled{false};
};

static bool is_esync(const std::shared_ptr<Camera>& d) {
    if (!d) return false;
    const char* n = d->Name();
    return n && std::string(n).find("eSync") != std::string::npos;
}

int main() {
    // Install signal handlers first so Ctrl+C is graceful.
    std::signal(SIGINT,  &on_signal);
    std::signal(SIGTERM, &on_signal);

    ShutdownGuard shutdown_guard;
    DeviceConnectionManager manager;
    TerminalModeGuard term_guard{true};

    // Fallback if std::exit() calls
    std::atexit([](){
        bool expected = false;
        if (!g_lib_shutdown_done.compare_exchange_strong(expected, true)) return;
        try { CameraManager::X().Shutdown(); } catch (...) {}
    });

    std::cout << "Searching for devices (press any key or Ctrl+C to stop)...\n";

    while (g_running.load(std::memory_order_acquire) && !keyPressed()) {
        const auto devices = manager.GetDevices();

        if (devices.empty()) {
            std::cout << "[Info] No devices detected.\n";
        } else {
            std::cout << "[Info] Connected devices:\n";
            for (const auto& dev : devices) {
                const bool es = is_esync(dev);
                std::cout << "  Name: " << (dev && dev->Name() ? dev->Name() : "(unknown)");
                if (es) std::cout << "  [eSync]";
                std::cout << "\n";
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "Stopping...\n";
    return 0;
}
