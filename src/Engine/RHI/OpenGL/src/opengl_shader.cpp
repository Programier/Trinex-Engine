#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <opengl_api.hpp>
#include <opengl_color_format.hpp>
#include <opengl_shader.hpp>


namespace Engine
{

    static const char* shader_type_name(GLuint shader)
    {
        switch (shader)
        {
            case GL_VERTEX_SHADER:
                return "vertex";
            case GL_FRAGMENT_SHADER:
                return "fragment";

            default:
                return "undefined";
        }
    }

    static GLuint convert_topology(PrimitiveTopology topology)
    {
        switch (topology)
        {
            case PrimitiveTopology::TriangleList:
                return GL_TRIANGLES;

            case PrimitiveTopology::PointList:
                return GL_POINTS;

            case PrimitiveTopology::LineList:
                return GL_LINES;

            case PrimitiveTopology::LineStrip:
                return GL_LINE_STRIP;

            case PrimitiveTopology::TriangleStrip:
                return GL_TRIANGLE_STRIP;

            case PrimitiveTopology::TriangleFan:
                return GL_TRIANGLE_FAN;

            case PrimitiveTopology::LineListWithAdjacency:
                return GL_LINES_ADJACENCY;

            case PrimitiveTopology::LineStripWithAdjacency:
                return GL_LINE_STRIP_ADJACENCY;

            case PrimitiveTopology::TriangleListWithAdjacency:
                return GL_TRIANGLES_ADJACENCY;

            case PrimitiveTopology::TriangleStripWithAdjacency:
                return GL_TRIANGLE_STRIP_ADJACENCY;

            case PrimitiveTopology::PatchList:
                return GL_PATCHES;

            default:
                break;
        }

        throw EngineException("Unsupported topology");
    }

    static GLuint compile_shader_module(const FileBuffer& shader_code, GLenum type, const String& name)
    {
        GLint shader_id = 0;
        GLuint program  = 0;

        auto cleanup = [&shader_id, &program]() {
            if (shader_id)
            {
                glDeleteShader(shader_id);
            }

            if (program)
            {
                glDeleteProgram(program);
            }

            shader_id = 0;
            program   = 0;
        };

        if (shader_code.empty())
            return shader_id;

        static GLchar log[1024];
        shader_id = glCreateShader(type);

        const GLchar* code = reinterpret_cast<const GLchar*>(shader_code.data());

        glShaderSource(shader_id, 1, &code, nullptr);
        glCompileShader(shader_id);

        GLint success;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader_id, 1024, nullptr, log);
            error_log("OpenGL Shader", "Failed to compile %s shader '%s'\n\n%s\n", shader_type_name(type), name.c_str(),
                      log);
            cleanup();
            return 0;
        }

        program = glCreateProgram();
        glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
        glAttachShader(program, shader_id);

        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(program, 1024, nullptr, log);
            error_log("Shader", "Failed to link shader program '%s'\n\n%s\n", name.c_str(), log);
            cleanup();
            return 0;
        }

        glDetachShader(program, shader_id);
        glDeleteShader(shader_id);

        return program;
    }

    OpenGL_Shader::~OpenGL_Shader()
    {
        if (_M_id)
        {
            glDeleteProgram(_M_id);
        }
    }

    GLuint OpenGL_VertexShader::type()
    {
        return GL_VERTEX_SHADER;
    }

    GLuint OpenGL_FragmentShader::type()
    {
        return GL_FRAGMENT_SHADER;
    }


    RHI_Shader* OpenGL::create_vertex_shader(const VertexShader* shader)
    {
        GLuint id = compile_shader_module(shader->text_code, GL_VERTEX_SHADER, shader->full_name());

        if (id)
        {
            OpenGL_Shader* vertex = new OpenGL_VertexShader();
            vertex->_M_id         = id;
            return vertex;
        }

        return nullptr;
    }

    RHI_Shader* OpenGL::create_fragment_shader(const FragmentShader* shader)
    {
        GLuint id = compile_shader_module(shader->text_code, GL_FRAGMENT_SHADER, shader->full_name());

        if (id)
        {
            OpenGL_Shader* fragment = new OpenGL_FragmentShader();
            fragment->_M_id         = id;
            return fragment;
        }

        return nullptr;
    }

    void OpenGL_Pipeline::init(const Pipeline* pipeline)
    {
        _M_topology = convert_topology(pipeline->input_assembly.primitive_topology);

        glGenVertexArrays(1, &_M_vao);
        glGenProgramPipelines(1, &_M_pipeline);

        if (pipeline->vertex_shader && pipeline->vertex_shader->rhi_object<OpenGL_Shader>())
        {
            glUseProgramStages(_M_pipeline, GL_VERTEX_SHADER_BIT,
                               pipeline->vertex_shader->rhi_object<OpenGL_Shader>()->_M_id);

            _M_vertex_input.reserve(pipeline->vertex_shader->attributes.size());

            for (auto& attribute : pipeline->vertex_shader->attributes)
            {
                VertexInput input;
                input.count =
                        static_cast<size_t>(attribute.count) * ColorFormatInfo::info_of(attribute.format).components();
                input.size = static_cast<size_t>(attribute.count) * ColorFormatInfo::info_of(attribute.format).size();
                input.type = color_format_from_engine_format(attribute.format)._M_type;

                auto metadata       = ColorFormatInfo::info_of(attribute.format).metadata();
                bool need_normalize = metadata == ColorFormatMetaData::Snorm || metadata == ColorFormatMetaData::Unorm;
                input.normalize     = need_normalize ? GL_TRUE : GL_FALSE;

                _M_vertex_input.push_back(input);
            }
        }

        if (pipeline->fragment_shader && pipeline->fragment_shader->rhi_object<OpenGL_Shader>())
        {
            glUseProgramStages(_M_pipeline, GL_FRAGMENT_SHADER_BIT,
                               pipeline->fragment_shader->rhi_object<OpenGL_Shader>()->_M_id);
        }
    }

    void OpenGL_Pipeline::bind()
    {
        if (OPENGL_API->_M_current_pipeline != this)
        {
            OPENGL_API->_M_current_pipeline = this;
            glBindProgramPipeline(_M_pipeline);
            glBindVertexArray(_M_vao);
        }
    }

    OpenGL_Pipeline::~OpenGL_Pipeline()
    {
        if (_M_pipeline)
        {
            glDeleteProgramPipelines(1, &_M_pipeline);
        }

        if (_M_vao)
        {
            glDeleteVertexArrays(1, &_M_vao);
        }
    }

    RHI_Pipeline* OpenGL::create_pipeline(const Pipeline* pipeline)
    {
        OpenGL_Pipeline* opengl_pipeline = new OpenGL_Pipeline();
        opengl_pipeline->init(pipeline);
        return opengl_pipeline;
    }
}// namespace Engine
