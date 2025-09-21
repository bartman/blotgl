#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <fmt/core.h>
#include <fmt/ostream.h>
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

#include "core_app.hpp"

namespace BlotGL {

class App : public Core::App {
protected:
    BlotGL::Frame<3> m_frame;

public:

    explicit App(unsigned width, unsigned height)
    : Core::App(width, height), m_frame{width, height}
    {}

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

}
