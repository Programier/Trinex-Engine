#pragma once
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>


namespace Engine
{
    struct OpenGL_Shader : public RHI_Shader {
        GLuint _M_id = 0;

        ~OpenGL_Shader();
        virtual GLuint type() = 0;
    };

    struct OpenGL_VertexShader : public OpenGL_Shader {
        virtual GLuint type() override;
    };

    struct OpenGL_FragmentShader : public OpenGL_Shader {
        virtual GLuint type() override;
    };


    struct OpenGL_Pipeline : public RHI_Pipeline {

        struct VertexInput {
            size_t count;
            size_t size;
            GLuint type;
            GLboolean normalize;
        };

        Vector<VertexInput> _M_vertex_input;

        GLuint _M_pipeline = 0;
        GLuint _M_topology = 0;
        GLuint _M_vao      = 0;

        void init(const Pipeline* pipeline);
        void bind() override;
        ~OpenGL_Pipeline();
    };
}// namespace Engine
