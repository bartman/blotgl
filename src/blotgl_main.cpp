#define GL_GLEXT_PROTOTYPES
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <compare>
#include <fmt/core.h>
#include <fmt/ostream.h>

extern "C" {
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>
};

#include "blot.hpp"
#include "blot_canvas.h"

int main() {
    const int width = 80;
    const int height = 40;
    const int bytes_per_pixel = 3;  // For 24-bit RGB

    // Open DRM render node (may need /dev/dri/card0; adjust if needed)
    int fd = open("/dev/dri/renderD128", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open render node (check permissions)\n");
        return 1;
    }

    // Create GBM device
    struct gbm_device *gbm = gbm_create_device(fd);
    if (!gbm) {
        fprintf(stderr, "gbm_create_device failed\n");
        close(fd);
        return 1;
    }

    // Get EGL display
    EGLDisplay dpy = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm, NULL);
    if (!dpy) {
        fprintf(stderr, "eglGetPlatformDisplay failed\n");
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    if (!eglInitialize(dpy, NULL, NULL)) {
        fprintf(stderr, "eglInitialize failed\n");
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Check required extensions
    const char *exts = eglQueryString(dpy, EGL_EXTENSIONS);
    if (!strstr(exts, "EGL_KHR_surfaceless_context")) {
        fprintf(stderr, "EGL_KHR_surfaceless_context not supported\n");
        eglTerminate(dpy);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Choose config
    static const EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(dpy, config_attribs, &config, 1, &num_configs) || num_configs == 0) {
        fprintf(stderr, "eglChooseConfig failed\n");
        eglTerminate(dpy);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Bind OpenGL API
    if (!eglBindAPI(EGL_OPENGL_API)) {
        fprintf(stderr, "eglBindAPI failed\n");
        eglTerminate(dpy);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Create context (OpenGL 3.3 core; adjust version as needed)
    static const EGLint ctx_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_NONE
    };
    EGLContext ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, ctx_attribs);
    if (!ctx) {
        fprintf(stderr, "eglCreateContext failed\n");
        eglTerminate(dpy);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Make surfaceless context current
    if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        eglDestroyContext(dpy, ctx);
        eglTerminate(dpy);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Allocate buffer for raw pixel data (top-to-bottom)
    unsigned char *pixels = (unsigned char *)malloc(width * height * bytes_per_pixel);
    if (!pixels) {
        fprintf(stderr, "Failed to allocate pixel buffer\n");
        eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(dpy, ctx);
        eglTerminate(dpy);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Create FBO and renderbuffer for offscreen rendering
    GLuint fbo, rb;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &rb);
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, width, height);  // 24-bit RGB
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer incomplete\n");
        // Cleanup...
        return 1;
    }

    // Set up viewport and clear
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Simple rendering: draw a colored triangle using modern shaders
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

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Link shaders into program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Define triangle vertices and colors
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

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Draw the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Cleanup shaders and buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Finish and read pixels
    glFinish();
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);  // For 24-bit; use GL_UNSIGNED_SHORT_5_6_5 for 16-bit

    // ------------------------------------------------------------------------

#define BRAILLE_GLYPH_BASE 0x2800

#define BRAILLE_GLYPH_ROWS 4
#define BRAILLE_GLYPH_COLS 2
#define BRAILLE_GLYPH_SIZE (BRAILLE_GLYPH_ROWS * BRAILLE_GLYPH_COLS)

#define BRAILLE_GLYPH_MAP_INDEX(x,y) ( ( (y) * BRAILLE_GLYPH_COLS) + (x) )

    #define DIV_ROUND_UP(n,by) (((n) + (by-1)) / (by))

    //   +------+------+
    //   | 0x01 | 0x08 |
    //   +------+------+
    //   | 0x02 | 0x10 |
    //   +------+------+
    //   | 0x04 | 0x20 |
    //   +------+------+
    //   | 0x40 | 0x80 |
    //   +------+------+

    std::array<uint8_t,BRAILLE_GLYPH_SIZE> b_mask = {
        0x01, 0x08,
        0x02, 0x10,
        0x04, 0x20,
        0x40, 0x80,
    };

    size_t b_cols = DIV_ROUND_UP(width, BRAILLE_GLYPH_COLS);
    size_t b_rows = DIV_ROUND_UP(height, BRAILLE_GLYPH_ROWS);
    size_t b_size = b_cols * b_rows;
    std::vector<uint8_t> braille(b_size, 0);

    struct color24 {
        uint8_t r{}, g{}, b{};

        operator bool() const { return (r|g|b) != 0; }
        auto operator<=>(const color24&) const = default;
    };
    std::vector<color24> colors(b_size, color24{});

    for (size_t y=0; y<height; y++) {
        for (size_t x=0; x<width; x++) {
            static_assert(bytes_per_pixel == 3);
            size_t pindex = (x + (y*width)) * bytes_per_pixel;
            color24 color{ pixels[pindex], pixels[pindex+1], pixels[pindex+2] };

            if (color) {
                size_t b_buf_index = (x/BRAILLE_GLYPH_COLS) + ((y/BRAILLE_GLYPH_ROWS)*b_cols);
                size_t b_sub_index = (x%BRAILLE_GLYPH_COLS) + ((y%BRAILLE_GLYPH_ROWS)*BRAILLE_GLYPH_COLS);

                assert(b_buf_index < b_size);
                assert(b_sub_index < BRAILLE_GLYPH_SIZE);

                braille[b_buf_index] |= b_mask[b_sub_index];
                colors[b_buf_index] = color;
            }
        }
    }

    auto print_unicode_char = [](std::ostream &out, char32_t codepoint) {
        if (codepoint <= 0x7F) {
            out << char(codepoint);
        } else if (codepoint <= 0x7FF) {
            out << char(0xC0 | ((codepoint >> 6) & 0x1F))
                << char(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0xFFFF) {
            out << char(0xE0 | ((codepoint >> 12) & 0x0F))
                << char(0x80 | ((codepoint >> 6) & 0x3F))
                << char(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0x10FFFF) {
            out << char(0xF0 | ((codepoint >> 18) & 0x07))
                << char(0x80 | ((codepoint >> 12) & 0x3F))
                << char(0x80 | ((codepoint >> 6) & 0x3F))
                << char(0x80 | (codepoint & 0x3F));
        }
    };

    auto print_color_code = [](std::ostream &out, color24 color) {
        out << std::format("\033[38;2;{};{};{}m", color.r, color.g, color.b);
    };

    std::stringstream ss;
    for (size_t row=0; row<b_rows; row++) {
        color24 color{};
        for (size_t col=0; col<b_cols; col++) {
            size_t b_buf_index = col + (row*b_cols);

            uint8_t b_glyph = braille[b_buf_index];
            if (!b_glyph) {
                ss << ' ';
                continue;
            }

            if (color != colors[b_buf_index]) {
                color = colors[b_buf_index];
                print_color_code(ss, color);
            }
            print_unicode_char(ss, BRAILLE_GLYPH_BASE + b_glyph);
        }
        if (color)
            ss << "\033[0m";

        ss << '\n';
    }
    std::puts(ss.str().c_str());

    // ------------------------------------------------------------------------

    // Cleanup GL objects
    glDeleteRenderbuffers(1, &rb);
    glDeleteFramebuffers(1, &fbo);

    // EGL cleanup
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(dpy, ctx);
    eglTerminate(dpy);
    gbm_device_destroy(gbm);
    close(fd);
    free(pixels);

    return 0;
}
