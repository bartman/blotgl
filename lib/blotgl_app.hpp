#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <stdexcept>

extern "C" {
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>
};

namespace BlotGL {

struct Event {
    // TODO
};

class App;

class Layer {
public:
    explicit Layer() = default;
    virtual ~Layer() = default;
    virtual void on_event(Event &event) {}
    virtual void on_update(const BlotGL::App &app, float timestamp) {}
    virtual void on_render() {}
};

class App final {
protected:
    unsigned m_width{};
    unsigned m_height{};
    int m_fd{-1};
    struct gbm_device *m_gbm{nullptr};
    EGLDisplay m_dpy{EGL_NO_DISPLAY};
    EGLContext m_ctx{EGL_NO_CONTEXT};
    GLuint m_fbo{0};
    GLuint m_rb{0};

    App(const App&) = delete;
    App(App&&) = delete;
    App& operator=(const App&) = delete;
    App& operator=(App&&) = delete;

    bool m_running = false;
    std::vector<std::unique_ptr<Layer>> m_layers;

    void step(float timestamp);

    static bool g_registered_sig_handler;
    static bool g_interrupted;
    static void register_sig_handler();
    static void sig_handler(int signo);

public:
    explicit App();
    ~App();

    std::pair<float,float> get_dimensions() const;

    int run();
    void stop();

    template <typename T>
    requires(std::is_base_of_v<Layer, T>)
    void push()
    {
        m_layers.push_back(std::make_unique<T>());
    }
};

}
