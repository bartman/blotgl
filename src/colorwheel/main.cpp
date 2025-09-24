#define GL_GLEXT_PROTOTYPES
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <chrono>
#include <thread>

extern "C" {
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
};

#include "app.hpp"

static bool interrupted = false;

static void sig_handler(int signo) {
    interrupted = true;
}

static void register_sig_handler() {
    // Add signal handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main() {
    App app(200, 100);

    auto start_time = std::chrono::steady_clock::now();
    auto last_time = start_time;
    double target_delta = 1.0 / 120.0;  // Cap at 120 FPS
    size_t frames = 0;

    while (!interrupted) {
        auto frame_start = std::chrono::steady_clock::now();
        app.frame();
        auto frame_end = std::chrono::steady_clock::now();

        auto delta = std::chrono::duration<double>(frame_end - start_time).count();
        auto avgsec = frames ? delta / frames : 0.0;
        auto fps = avgsec ? 1.0 / avgsec : 0.0;
        fmt::print("FPS: {:.2f}\n", fps);

        auto current_time = std::chrono::steady_clock::now();
        if (target_delta) {
            auto elapsed = std::chrono::duration<double>(current_time - frame_start).count();
            if (elapsed < target_delta) {
                std::this_thread::sleep_for(std::chrono::duration<double>(target_delta - elapsed));
            }
        }

        last_time = current_time;
        frames ++;
    }

    return 0;
}
