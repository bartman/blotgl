#define GL_GLEXT_PROTOTYPES
#include "blotgl_app.hpp"
#include "blotgl_frame.hpp"
#include "blotgl_terminal.hpp"
#include "blotgl_braille.hpp"
#include "blotgl_glerror.hpp"

#include <chrono>
#include <thread>

extern "C" {
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
};

namespace BlotGL {

App::App()
{
    update_dimensions();

    m_fd = open("/dev/dri/renderD128", O_RDWR);
    if (m_fd < 0) {
        fprintf(stderr, "Failed to open render node (check permissions)\n");
        throw std::runtime_error("Failed to open render node");
    }

    m_gbm = gbm_create_device(m_fd);
    if (!m_gbm) {
        fprintf(stderr, "gbm_create_device failed\n");
        close(m_fd);
        throw std::runtime_error("gbm_create_device failed");
    }

    m_dpy = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, m_gbm, NULL);
    if (!m_dpy) {
        fprintf(stderr, "eglGetPlatformDisplay failed\n");
        gbm_device_destroy(m_gbm);
        close(m_fd);
        throw std::runtime_error("eglGetPlatformDisplay failed");
    }

    if (!eglInitialize(m_dpy, NULL, NULL)) {
        fprintf(stderr, "eglInitialize failed\n");
        eglTerminate(m_dpy);
        gbm_device_destroy(m_gbm);
        close(m_fd);
        throw std::runtime_error("eglInitialize failed");
    }

    const char *exts = eglQueryString(m_dpy, EGL_EXTENSIONS);
    if (!strstr(exts, "EGL_KHR_surfaceless_context")) {
        fprintf(stderr, "EGL_KHR_surfaceless_context not supported\n");
        eglTerminate(m_dpy);
        gbm_device_destroy(m_gbm);
        close(m_fd);
        throw std::runtime_error("EGL_KHR_surfaceless_context not supported");
    }

    static const EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(m_dpy, config_attribs, &config, 1, &num_configs) || num_configs == 0) {
        fprintf(stderr, "eglChooseConfig failed\n");
        eglTerminate(m_dpy);
        gbm_device_destroy(m_gbm);
        close(m_fd);
        throw std::runtime_error("eglChooseConfig failed");
    }

    if (!eglBindAPI(EGL_OPENGL_API)) {
        fprintf(stderr, "eglBindAPI failed\n");
        eglTerminate(m_dpy);
        gbm_device_destroy(m_gbm);
        close(m_fd);
        throw std::runtime_error("eglBindAPI failed");
    }

    static const EGLint ctx_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_NONE
    };
    m_ctx = eglCreateContext(m_dpy, config, EGL_NO_CONTEXT, ctx_attribs);
    if (!m_ctx) {
        fprintf(stderr, "eglCreateContext failed\n");
        eglTerminate(m_dpy);
        gbm_device_destroy(m_gbm);
        close(m_fd);
        throw std::runtime_error("eglCreateContext failed");
    }

    if (!eglMakeCurrent(m_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, m_ctx)) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        eglDestroyContext(m_dpy, m_ctx);
        eglTerminate(m_dpy);
        gbm_device_destroy(m_gbm);
        close(m_fd);
        throw std::runtime_error("eglMakeCurrent failed");
    }

    GL(glGenFramebuffers(1, &m_fbo));
    GL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

    GL(glGenRenderbuffers(1, &m_rb));
    GL(glBindRenderbuffer(GL_RENDERBUFFER, m_rb));
    GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, m_width, m_height));
    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_rb));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer incomplete\n");
        glDeleteRenderbuffers(1, &m_rb);
        glDeleteFramebuffers(1, &m_fbo);
        eglMakeCurrent(m_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(m_dpy, m_ctx);
        eglTerminate(m_dpy);
        gbm_device_destroy(m_gbm);
        close(m_fd);
        throw std::runtime_error("Framebuffer incomplete");
    }

    if (blotgl_drain_glerrors()) {
        glDeleteRenderbuffers(1, &m_rb);
        glDeleteFramebuffers(1, &m_fbo);
        eglMakeCurrent(m_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(m_dpy, m_ctx);
        eglTerminate(m_dpy);
        gbm_device_destroy(m_gbm);
        close(m_fd);
        throw std::runtime_error("OpenGL errors during init");
    }
}

App::~App() {
    glDeleteRenderbuffers(1, &m_rb);
    glDeleteFramebuffers(1, &m_fbo);
    eglMakeCurrent(m_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_dpy, m_ctx);
    eglTerminate(m_dpy);
    gbm_device_destroy(m_gbm);
    close(m_fd);
}

bool App::update_dimensions()
{
    unsigned cols, rows;
    try {
        auto ws = linux_terminal_winsize();
        cols = (ws.ws_col-1) * BRAILLE_GLYPH_COLS;
        rows = (ws.ws_row-1) * BRAILLE_GLYPH_ROWS;
    } catch (std::exception ex) {}

    // smallest size is 200x100, and must be multiple of braille size
    cols = std::max(200u, multiple_of<unsigned>(cols, BRAILLE_GLYPH_COLS));
    rows = std::max(100u, multiple_of<unsigned>(rows, BRAILLE_GLYPH_ROWS));

    if (cols == m_width && rows == m_height)
        return false;

    glDeleteRenderbuffers(1, &m_rb);

    GL(glGenRenderbuffers(1, &m_rb));
    GL(glBindRenderbuffer(GL_RENDERBUFFER, m_rb));
    GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, m_width, m_height));
    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_rb));

    m_width = cols;
    m_height = rows;
    return true;
}

std::pair<float,float> App::get_dimensions() const
{
    return { m_width, m_height };
}

// TODO: should do proper event handling

bool App::g_registered_sig_handler = false;
bool App::g_interrupted = false;

void App::register_sig_handler() {
    if (g_registered_sig_handler)
        return;
    struct sigaction sa{};
    sa.sa_handler = App::sig_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    g_registered_sig_handler = true;
}

void App::sig_handler(int signo) {
    g_interrupted = true;
}

int App::run()
{
    auto start_time = std::chrono::steady_clock::now();
    auto last_time = start_time;
    double target_delta = 1.0 / 120.0;  // Cap at 120 FPS
    size_t frames = 0;

    m_running = true;

    if (blotgl_drain_glerrors())
        return 1;

    while (m_running) {
        auto frame_start = std::chrono::steady_clock::now();
        auto timestamp = std::chrono::duration<double>(frame_start - start_time).count();
        step(timestamp);
        auto frame_end = std::chrono::steady_clock::now();

        auto delta = std::chrono::duration<double>(frame_end - start_time).count();
        auto avgsec = frames ? delta / frames : 0.0;
        auto fps = avgsec ? 1.0 / avgsec : 0.0;
        fmt::print("{}x{} FPS: {:.2f}\n", m_width, m_height, fps);

        if (g_interrupted)
            return 1;

        if (blotgl_drain_glerrors())
            return 1;

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

void App::step(float timestamp)
{
    update_dimensions();

    GL(glViewport(0, 0, m_width, m_height));
    GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GL(glClear(GL_COLOR_BUFFER_BIT));

    Frame<3> frame(m_width, m_height);

    for (const auto &layer : m_layers)
        layer->on_update(*this, timestamp);

    for (const auto &layer : m_layers)
        layer->on_render();

    GL(glFinish());
    GL(glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, frame.pixels()));

    frame.pixels_to_braille(true);
    std::stringstream ss;
    frame.braille_to_stream(ss);
    std::puts(ss.str().c_str());
}

}
