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
    bool m_check_status{true};

    static void check_shader_status(GLuint part, GLuint stage, const char *desc)
    {
        GLint status = 0;
        glGetShaderiv(part, stage, &status);
        if (status == GL_FALSE) {
            GLint log_length = 0;
            glGetShaderiv(part, GL_INFO_LOG_LENGTH, &log_length);
            if (log_length > 0) {
                std::string log;
                log.resize(log_length);
                glGetShaderInfoLog(part, log_length, NULL, log.data());
                throw std::runtime_error(
                    std::format("{} Error:\n{}", desc, log));
            }
        }
    }

    static void check_program_status(GLuint part, GLuint stage, const char *desc)
    {
        GLint status = 0;
        glGetProgramiv(part, stage, &status);
        if (status == GL_FALSE) {
            GLint log_length = 0;
            glGetProgramiv(part, GL_INFO_LOG_LENGTH, &log_length);
            if (log_length > 0) {
                std::string log;
                log.resize(log_length);
                glGetShaderInfoLog(part, log_length, NULL, log.data());
                throw std::runtime_error(
                    std::format("{} Error:\n{}", desc, log));
            }
        }
    }


public:
    explicit Shader(const char *vertex_shader_source,
                    const char *fragment_shader_source) {
        m_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        GL(glShaderSource(m_vertex_shader, 1, &vertex_shader_source, NULL));
        GL(glCompileShader(m_vertex_shader));

        check_shader_status(m_vertex_shader, GL_COMPILE_STATUS, "Vertex Shader Compile");

        m_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        GL(glShaderSource(m_fragment_shader, 1, &fragment_shader_source, NULL));
        GL(glCompileShader(m_fragment_shader));

        check_shader_status(m_fragment_shader, GL_COMPILE_STATUS, "Fragment Shader Compile");

        m_shader_program = glCreateProgram();
        GL(glAttachShader(m_shader_program, m_vertex_shader));
        GL(glAttachShader(m_shader_program, m_fragment_shader));
        GL(glLinkProgram(m_shader_program));

        check_program_status(m_shader_program, GL_LINK_STATUS, "Shader Program Link");
    }

    ~Shader() {
        GL(glDeleteProgram(m_shader_program));
        GL(glDeleteShader(m_vertex_shader));
        GL(glDeleteShader(m_fragment_shader));
    }

    void use() {
        if (m_check_status) {
            check_program_status(m_shader_program, GL_VALIDATE_STATUS, "Shader Program Validation");
            m_check_status = false;
        }
        GL(glUseProgram(m_shader_program));
    }

};

}

