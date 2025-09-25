#include <format>
#include <fmt/core.h>
#include <fmt/ostream.h>

extern "C" {
#include <GL/gl.h>
#include <GL/glext.h>
};

#include "blotgl_glerror.hpp"

std::vector<std::string> g_blotgl_glerrors;

void blotgl_check_glerror(const char* operation) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        fmt::println(stderr, "OpenGL error after {}: {}", operation, err);
    }
}
void blotgl_push_glerror(const char* operation) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        g_blotgl_glerrors.push_back(
            std::format("OpenGL error after {}: {}", operation, err));
    }
}
void blotgl_drain_glerrors() {
    for (const auto &err : g_blotgl_glerrors) {
        fmt::println(stderr, "{}", err);
    }
    g_blotgl_glerrors.clear();
}
