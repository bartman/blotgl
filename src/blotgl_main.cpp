// filepath: src/blotgl_main.cpp
// ...existing code... (replace entire file content with the following refactored version)

#define GL_GLEXT_PROTOTYPES
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <compare>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <stdexcept>
#include <sstream>

extern "C" {
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>
};

#include "blotgl_frame.hpp"
#include "blotgl_braille.hpp"
#include "blotgl_color.hpp"

class App {
private:
    int m_width = 200;
    int m_height = 100;
    BlotGL::Frame<3> m_frame;
    int m_fd;
    struct gbm_device *m_gbm;
    EGLDisplay m_dpy;
    EGLContext m_ctx;
    GLuint m_fbo;
    GLuint m_rb;

public:
    App()
        : m_frame(m_width, m_height), m_fd(-1), m_gbm(nullptr), m_dpy(EGL_NO_DISPLAY), m_ctx(EGL_NO_CONTEXT), m_fbo(0), m_rb(0) {
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

    void frame() {
        glViewport(0, 0, m_width, m_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        const char *vertexShaderSource = R"glsl(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec3 aColor;
            out vec3 fragColor;
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                fragColor = aColor;
            }
        )glsl";

        const char *fragmentShaderSource = R"glsl(
            #version 330 core
            in vec3 fragColor;
            out vec4 FragColor;
            void main() {
                FragColor = vec4(fragColor, 1.0);
            }
        )glsl";

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        float vertices[] = {
            -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  // Red
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Green
             0.0f,  0.5f, 0.0f, 0.0f, 1.0f   // Blue
        };

        GLuint VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glFinish();
        glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_frame.pixels());

        m_frame.pixels_to_braille();
        std::stringstream ss;
        m_frame.braille_to_stream(ss);
        std::puts(ss.str().c_str());
    }
};

int main() {
    App app;
    app.frame();
    return 0;
}
