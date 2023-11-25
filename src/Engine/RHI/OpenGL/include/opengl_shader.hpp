#pragma once
#include <Core/executable_object.hpp>
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


    template<typename Func>
    struct OpenGL_StateCommand : public ExecutableObject {
        Func _M_func;

        OpenGL_StateCommand(Func func) : _M_func(func)
        {}

        int_t execute() override
        {
            _M_func();
            return 0;
        }
    };

    struct OpenGL_Pipeline : public RHI_Pipeline {

        struct VertexInput {
            size_t count;
            size_t size;
            GLuint type;
            GLboolean normalize;
        };

        Vector<VertexInput> _M_vertex_input;
        Vector<ExecutableObject*> _M_apply_state;

        GLuint _M_pipeline = 0;
        GLuint _M_topology = 0;
        GLuint _M_vao      = 0;

        void init(const Pipeline* pipeline);
        void bind() override;
        ~OpenGL_Pipeline();
    };
}// namespace Engine
