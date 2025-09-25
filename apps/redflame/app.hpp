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

#include <glm/glm.hpp>

#include "blotgl_frame.hpp"
#include "blotgl_shader.hpp"
#include "blotgl_app.hpp"
#include "blotgl_mmapped_file.hpp"

class AppLayer : public BlotGL::Layer {
protected:
    BlotGL::MmappedFile m_vertex_shader_source;
    BlotGL::MmappedFile m_fragment_shader_source;

    BlotGL::Shader m_shader;
    uint32_t m_vertex_array = 0;
	uint32_t m_vertex_buffer = 0;

public:

    explicit AppLayer()
    : BlotGL::Layer(),
        m_vertex_shader_source("apps/redflame/vertex.glsl"),
        m_fragment_shader_source("apps/redflame/fragment.glsl"),
        m_shader(m_vertex_shader_source.str().c_str(), m_fragment_shader_source.str().c_str())
    {
        // Create geometry
        glCreateVertexArrays(1, &m_vertex_array);
        glCreateBuffers(1, &m_vertex_buffer);

        struct Vertex
        {
            glm::vec2 Position;
            glm::vec2 TexCoord;
        };

        Vertex vertices[] = {
            { {-1.0f, -1.0f }, { 0.0f, 0.0f } },  // Bottom-left
            { { 3.0f, -1.0f }, { 2.0f, 0.0f } },  // Bottom-right
            { {-1.0f,  3.0f }, { 0.0f, 2.0f } }   // Top-left
        };

        glNamedBufferData(m_vertex_buffer, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Bind the VBO to VAO at binding index 0
        glVertexArrayVertexBuffer(m_vertex_array, 0, m_vertex_buffer, 0, sizeof(Vertex));

        // Enable attributes
        glEnableVertexArrayAttrib(m_vertex_array, 0); // position
        glEnableVertexArrayAttrib(m_vertex_array, 1); // uv

        // Format: location, size, type, normalized, relative offset
        glVertexArrayAttribFormat(m_vertex_array, 0, 2, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(Vertex, Position)));
        glVertexArrayAttribFormat(m_vertex_array, 1, 2, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(Vertex, TexCoord)));

        // Link attribute locations to binding index 0
        glVertexArrayAttribBinding(m_vertex_array, 0, 0);
        glVertexArrayAttribBinding(m_vertex_array, 1, 0);
    }
    ~AppLayer()
    {
        glDeleteVertexArrays(1, &m_vertex_array);
        glDeleteBuffers(1, &m_vertex_buffer);
    }

    void on_update(const BlotGL::App &app, float timestamp) override
    {
        m_shader.use();

        // Uniforms
        glUniform1f(0, timestamp);

        const auto &[width,height] = app.get_dimensions();
        glUniform2f(1, width, height);

        // Render
        glBindVertexArray(m_vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);

    }
};
