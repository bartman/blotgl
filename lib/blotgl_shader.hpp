#pragma once
#include <cassert>
#include <ostream>
#include <vector>
#include <cstdint>
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

#include "blotgl_glerror.hpp"

namespace BlotGL {

class Shader final {
protected:
    GLuint m_fragment_shader{};
    GLuint m_vertex_shader{};
    GLuint m_shader_program{};

public:
    explicit Shader(const char *vertex_shader_source,
                    const char *fragment_shader_source) {
        m_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        GL(glShaderSource(m_vertex_shader, 1, &vertex_shader_source, NULL));
        GL(glCompileShader(m_vertex_shader));

        m_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        GL(glShaderSource(m_fragment_shader, 1, &fragment_shader_source, NULL));
        GL(glCompileShader(m_fragment_shader));

        m_shader_program = glCreateProgram();
        GL(glAttachShader(m_shader_program, m_vertex_shader));
        GL(glAttachShader(m_shader_program, m_fragment_shader));
        GL(glLinkProgram(m_shader_program));
    }

    ~Shader() {
        GL(glDeleteProgram(m_shader_program));
        GL(glDeleteShader(m_vertex_shader));
        GL(glDeleteShader(m_fragment_shader));
    }

    void use() {
        GL(glUseProgram(m_shader_program));
    }

};

}

