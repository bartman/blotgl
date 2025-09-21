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

namespace Core {

class App {
protected:
    unsigned m_width;
    unsigned m_height;
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

public:
    explicit App(unsigned width, unsigned height)
        : m_width{width}, m_height{height} {
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

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        glGenRenderbuffers(1, &m_rb);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_rb);

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
    }

    ~App() {
        glDeleteRenderbuffers(1, &m_rb);
        glDeleteFramebuffers(1, &m_fbo);
        eglMakeCurrent(m_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(m_dpy, m_ctx);
        eglTerminate(m_dpy);
        gbm_device_destroy(m_gbm);
        close(m_fd);
    }

};

}
