#include <format>
#include <fmt/core.h>
#include <fmt/ostream.h>

extern "C" {
#include <GL/gl.h>
#include <GL/glext.h>
};

#include "blotgl_glerror.hpp"

std::vector<std::string> g_blotgl_glerrors;

const char* glErrorToString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default: return "Unknown GL error";
    }
}

size_t blotgl_check_glerror(const char* operation) {
    GLenum err;
    size_t count{};
    while ((err = glGetError()) != GL_NO_ERROR) {
        fmt::println(stderr, "OpenGL error after {}: {}: {}", operation, err, glErrorToString(err));
        count ++;
    }
    return count;
}
void blotgl_push_glerror(const char* operation) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        g_blotgl_glerrors.push_back(
            std::format("OpenGL error after {}: {}: {}", operation, err, glErrorToString(err)));
    }
}
size_t blotgl_drain_glerrors() {
    size_t count{};
    for (const auto &err : g_blotgl_glerrors) {
        fmt::println(stderr, "{}", err);
        count ++;
    }
    g_blotgl_glerrors.clear();
    return count;
}
