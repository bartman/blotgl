#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
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

#include "blotgl_frame.hpp"
#include "blotgl_shader.hpp"
#include "blotgl_app.hpp"
#include "blotgl_glerror.hpp"

class AppLayer : public BlotGL::Layer {
protected:
    static constexpr const char *vertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec3 aColor;
        out vec3 fragColor;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            fragColor = aColor;
        }
    )glsl";

    static constexpr const char *fragmentShaderSource = R"glsl(
        #version 330 core
        in vec3 fragColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(fragColor, 1.0);
        }
    )glsl";

    BlotGL::Shader m_shader;

    size_t m_color_count{};
    size_t m_vertex_count{};

    std::vector<float> m_vertices;
    float m_degrees = 0;

public:

    explicit AppLayer()
    : BlotGL::Layer(), m_shader(vertexShaderSource, fragmentShaderSource)
    {
        build_triangles();
    }
    ~AppLayer()
    {
    }

    void build_triangles() {
        const size_t ccount = 12;
        const double radius = 0.95;

        m_color_count = ccount;
        float colors[ccount*3] {
            1.00f, 0.00f, 0.00f, // R
            0.66f, 0.33f, 0.00f, // ry
            0.50f, 0.50f, 0.00f, // Y
            0.33f, 0.66f, 0.00f, // yg
            0.00f, 1.00f, 0.00f, // G
            0.00f, 0.66f, 0.33f, // gc
            0.00f, 0.50f, 0.50f, // C
            0.00f, 0.33f, 0.66f, // cb
            0.00f, 0.00f, 1.00f, // B
            0.33f, 0.00f, 0.66f, // bm
            0.50f, 0.00f, 0.50f, // M
            0.66f, 0.00f, 0.33f, // mr
        };

        m_vertex_count = ccount * 3;
        m_vertices.resize(m_vertex_count * 5);
        float *vertices = m_vertices.data();

        for (int c = 0; c < m_color_count; ++c) {
            int vi = c*15;

            // one
            int c0 = c * 3;
            int v0 = c * 15;
            float r0 = (c * 30) * static_cast<float>(M_PI) / 180.0f;
            vertices[v0+0] = radius * cosf(r0);
            vertices[v0+1] = radius * sinf(r0);
            vertices[v0+2] = colors[c0+0];
            vertices[v0+3] = colors[c0+1];
            vertices[v0+4] = colors[c0+2];

            // two
            int c1 = ((c+1)%m_color_count)*3;
            int v1 = v0 + 5;
            float r1 = ((c+1) * 30) * static_cast<float>(M_PI) / 180.0f;
            vertices[v1+0] = radius * cosf(r1);
            vertices[v1+1] = radius * sinf(r1);
            vertices[v1+2] = colors[c1+0];
            vertices[v1+3] = colors[c1+1];
            vertices[v1+4] = colors[c1+2];

            // three
            int v2 = v1 + 5;
            vertices[v2+0] = 0;
            vertices[v2+1] = 0;
            vertices[v2+2] = 1;
            vertices[v2+3] = 1;
            vertices[v2+4] = 1;

        }
    }

    void on_update(const BlotGL::App &app, float timestamp) override
    {
        m_shader.use();

        float vertices[m_vertices.size()];
        memcpy(vertices, m_vertices.data(), sizeof(vertices));

        float radians = m_degrees * static_cast<float>(M_PI) / 180.0f;
        for (int v = 0; v < m_vertex_count; ++v) {
            int i = 5*v;
            float new_x = vertices[i+0] * cosf(radians) - vertices[i+1] * sinf(radians);
            float new_y = vertices[i+0] * sinf(radians) + vertices[i+1] * cosf(radians);
            vertices[i+0] = new_x;
            vertices[i+1] = new_y;
        }
        m_degrees += 1;

        GLuint VBO, VAO;
        GL(glGenVertexArrays(1, &VAO));
        GL(glGenBuffers(1, &VBO));

        GL(glBindVertexArray(VAO));
        GL(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

        GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
        GL(glEnableVertexAttribArray(0));
        GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float))));
        GL(glEnableVertexAttribArray(1));

        for (int c = 0; c < m_color_count; ++c)
            GL(glDrawArrays(GL_TRIANGLES, c*3, 3));

        GL(glDeleteVertexArrays(1, &VAO));
        GL(glDeleteBuffers(1, &VBO));
    }
};
