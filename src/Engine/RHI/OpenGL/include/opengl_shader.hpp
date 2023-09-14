#pragma once
#include <Core/shader_types.hpp>

#include <opengl_command_buffer.hpp>
#include <opengl_object.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
    struct OpenGL_Shader : OpenGL_Object {
        GLuint _M_vertex   = 0;
        GLuint _M_fragment = 0;
        GLenum _M_topology = 0;
        OpenGL_CommandBuffer _M_command_buffer;

        Vector<VertexAttribute> _M_attributes;
        uint_t _M_vertex_size = 0;

        Map<BindingIndex, GLuint> _M_block_indices;

        implement_opengl_instance_hpp();

        OpenGL_Shader& apply_vertex_attributes(ArrayOffset offset);

        ~OpenGL_Shader();
    };
}// namespace Engine
