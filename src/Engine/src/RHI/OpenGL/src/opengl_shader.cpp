#include <Graphics/pipeline.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/shader.hpp>
#include <opengl_api.hpp>
#include <opengl_color_format.hpp>
#include <opengl_enums_convertor.hpp>
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

    static GLuint compile_shader_module(const Buffer& shader_code, GLenum type, const String& name)
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
            error_log("OpenGL Shader", "Failed to compile %s shader '%s'\n\n%s\n", shader_type_name(type), name.c_str(), log);
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
        if (m_id)
        {
            glDeleteProgram(m_id);
        }
    }

    GLuint OpenGL_VertexShader::type()
    {
        return GL_VERTEX_SHADER;
    }

    GLuint OpenGL_TesselationControlShader::type()
    {
        return GL_TESS_CONTROL_SHADER;
    }

    GLuint OpenGL_TesselationShader::type()
    {
        return GL_TESS_EVALUATION_SHADER;
    }

    GLuint OpenGL_GeometryShader::type()
    {
        return GL_GEOMETRY_SHADER;
    }

    GLuint OpenGL_FragmentShader::type()
    {
        return GL_FRAGMENT_SHADER;
    }


    template<typename ShaderType, GLenum type>
    static RHI_Shader* create_opengl_shader(const Shader* shader)
    {
        GLuint id = compile_shader_module(shader->source_code, type, shader->full_name());

        if (id)
        {
            ShaderType* opengl_shader = new ShaderType();
            opengl_shader->m_id       = id;
            return opengl_shader;
        }

        return nullptr;
    }

    RHI_Shader* OpenGL::create_vertex_shader(const VertexShader* shader)
    {
        return create_opengl_shader<OpenGL_VertexShader, GL_VERTEX_SHADER>(shader);
    }

    RHI_Shader* OpenGL::create_fragment_shader(const FragmentShader* shader)
    {
        return create_opengl_shader<OpenGL_FragmentShader, GL_FRAGMENT_SHADER>(shader);
    }

    RHI_Shader* OpenGL::create_tesselation_control_shader(const TessellationControlShader* shader)
    {
        return create_opengl_shader<OpenGL_TesselationControlShader, GL_TESS_CONTROL_SHADER>(shader);
    }

    RHI_Shader* OpenGL::create_tesselation_shader(const TessellationShader* shader)
    {
        return create_opengl_shader<OpenGL_TesselationShader, GL_TESS_EVALUATION_SHADER>(shader);
    }

    RHI_Shader* OpenGL::create_geometry_shader(const GeometryShader* shader)
    {
        return create_opengl_shader<OpenGL_GeometryShader, GL_GEOMETRY_SHADER>(shader);
    }


    static ExecutableObject* apply_stencil_face(GLuint face, const Pipeline::StencilTestInfo::FaceInfo& face_info)
    {
        GLuint fail         = stencil_op(face_info.fail);
        GLuint depth_pass   = stencil_op(face_info.depth_pass);
        GLuint depth_fail   = stencil_op(face_info.depth_fail);
        GLuint compare      = compare_func(face_info.compare);
        GLint ref           = face_info.reference;
        GLuint compare_mask = face_info.compare_mask;
        GLuint write_mask   = face_info.write_mask;

        return new OpenGL_StateCommand([=]() {
            glStencilMaskSeparate(face, write_mask);
            glStencilFuncSeparate(face, compare, ref, compare_mask);
            glStencilOpSeparate(face, fail, depth_fail, depth_pass);
        });
    }

    template<typename Func, typename... Args>
    static ExecutableObject* wrap_command(Func func, Args... args)
    {
        return new OpenGL_StateCommand([=]() { func(args...); });
    }

#define new_command(...) m_apply_state.push_back(wrap_command(__VA_ARGS__))
#define new_command_nowrap(...) m_apply_state.push_back(__VA_ARGS__)

    void OpenGL_Pipeline::init_pipeline_shader(Shader* shader, GLbitfield stage)
    {
        if (shader && shader->rhi_object<OpenGL_Shader>())
        {
            glUseProgramStages(m_pipeline, stage, shader->rhi_object<OpenGL_Shader>()->m_id);
        }
    }

    static FORCE_INLINE void parse_vertex_input(OpenGL_Pipeline::VertexInput& out, const VertexShader::Attribute& attribute)
    {
        switch (attribute.type)
        {
            case VertexBufferElementType::Float1:
                out.count      = 1;
                out.size       = sizeof(float) * out.count;
                out.type       = GL_FLOAT;
                out.normalized = GL_FALSE;
                break;
            case VertexBufferElementType::Float2:
                out.count      = 2;
                out.size       = sizeof(float) * out.count;
                out.type       = GL_FLOAT;
                out.normalized = GL_FALSE;
                break;
            case VertexBufferElementType::Float3:
                out.count      = 3;
                out.size       = sizeof(float) * out.count;
                out.type       = GL_FLOAT;
                out.normalized = GL_FALSE;
                break;
            case VertexBufferElementType::Float4:
                out.count      = 4;
                out.size       = sizeof(float) * out.count;
                out.type       = GL_FLOAT;
                out.normalized = GL_FALSE;
                break;
            case VertexBufferElementType::UByte4:
                out.count      = 4;
                out.size       = sizeof(byte) * out.count;
                out.type       = GL_UNSIGNED_BYTE;
                out.normalized = GL_FALSE;
                break;
            case VertexBufferElementType::UByte4N:
                out.count      = 4;
                out.size       = sizeof(byte) * out.count;
                out.type       = GL_UNSIGNED_BYTE;
                out.normalized = GL_TRUE;
                break;
            case VertexBufferElementType::Color:
                out.count      = 4;
                out.size       = sizeof(byte) * out.count;
                out.type       = GL_UNSIGNED_BYTE;
                out.normalized = GL_TRUE;
                break;
            default:
                break;
        }
    }

    void OpenGL_Pipeline::init(const Pipeline* pipeline)
    {
        m_topology          = convert_topology(pipeline->input_assembly.primitive_topology);
        m_global_parameters = pipeline->global_parameters;
        m_local_parameters  = pipeline->local_parameters;

        glGenVertexArrays(1, &m_vao);
        glGenProgramPipelines(1, &m_pipeline);
        glBindVertexArray(m_vao);

        auto vertex_shader   = pipeline->vertex_shader();
        auto fragment_shader = pipeline->fragment_shader();

        if (vertex_shader->rhi_object<OpenGL_Shader>())
        {
            glUseProgramStages(m_pipeline, GL_VERTEX_SHADER_BIT, vertex_shader->rhi_object<OpenGL_Shader>()->m_id);

            m_vertex_input.reserve(vertex_shader->attributes.size());

            GLuint index = 0;
            for (auto& attribute : vertex_shader->attributes)
            {
                glEnableVertexAttribArray(index);
                VertexInput input;
                parse_vertex_input(input, attribute);

                if (attribute.rate == VertexAttributeInputRate::Instance)
                {
                    glVertexAttribDivisor(index, 1);
                }

                m_vertex_input.push_back(input);
                ++index;
            }
        }


        glBindVertexArray(0);
        init_pipeline_shader(pipeline->tessellation_control_shader(), GL_TESS_CONTROL_SHADER_BIT);
        init_pipeline_shader(pipeline->tessellation_shader(), GL_TESS_EVALUATION_SHADER_BIT);
        init_pipeline_shader(pipeline->geometry_shader(), GL_GEOMETRY_SHADER_BIT);
        init_pipeline_shader(fragment_shader, GL_FRAGMENT_SHADER_BIT);


        /// Initialize pipeline state
        // Depth test

        new_command((pipeline->depth_test.enable ? glEnable : glDisable), GL_DEPTH_TEST);
        new_command(glDepthMask, static_cast<GLboolean>(pipeline->depth_test.write_enable));
        new_command(glDepthFunc, depth_func(pipeline->depth_test.func));

#if USING_OPENGL_CORE
        new_command((pipeline->input_assembly.primitive_restart_enable ? glEnable : glDisable), GL_PRIMITIVE_RESTART);
#else
        new_command((pipeline->input_assembly.primitive_restart_enable ? glEnable : glDisable), GL_PRIMITIVE_RESTART_FIXED_INDEX);
#endif


        // Stencil test
        new_command((pipeline->stencil_test.enable ? glEnable : glDisable), GL_STENCIL_TEST);
        new_command_nowrap(apply_stencil_face(GL_FRONT, pipeline->stencil_test.front));
        new_command_nowrap(apply_stencil_face(GL_BACK, pipeline->stencil_test.back));


        // Rasterization
#if USING_OPENGL_CORE
        if (pipeline->rasterizer.depth_clamp_enable)
        {
            new_command(glEnable, static_cast<GLenum>(GL_DEPTH_CLAMP));
        }
        else

        {
            new_command(glDisable, static_cast<GLenum>(GL_DEPTH_CLAMP));
        }
#endif


        if (pipeline->rasterizer.depth_bias_enable)
        {
            new_command(glEnable, static_cast<GLenum>(GL_POLYGON_OFFSET_FILL));
            new_command(glPolygonOffset, pipeline->rasterizer.depth_bias_const_factor,
                        pipeline->rasterizer.depth_bias_slope_factor);
        }
        else
        {
            new_command(glDisable, static_cast<GLenum>(GL_POLYGON_OFFSET_FILL));
        }

        new_command(glLineWidth, pipeline->rasterizer.line_width);

#if USING_OPENGL_CORE
        new_command(glPolygonMode, GL_FRONT_AND_BACK, polygon_mode(pipeline->rasterizer.polygon_mode));
#endif


        if (pipeline->rasterizer.cull_mode != CullMode::None)
        {
            new_command(glEnable, static_cast<GLenum>(GL_CULL_FACE));
            new_command(glCullFace, cull_mode(pipeline->rasterizer.cull_mode));
        }
        else
        {
            new_command(glDisable, static_cast<GLenum>(GL_CULL_FACE));
        }

        new_command(glFrontFace, front_face(pipeline->rasterizer.front_face));

        // Blending

#if USING_OPENGL_CORE
        if (pipeline->color_blending.logic_op_enable)
        {
            new_command(glEnable, GL_COLOR_LOGIC_OP);
            new_command(glLogicOp, logic_op(pipeline->color_blending.logic_op));
        }
        else
        {
            new_command(glDisable, GL_COLOR_LOGIC_OP);
        }
#endif

        new_command(glBlendColor, pipeline->color_blending.blend_constants.r, pipeline->color_blending.blend_constants.g,
                    pipeline->color_blending.blend_constants.b, pipeline->color_blending.blend_constants.a);

        RenderPass* render_pass = pipeline->render_pass();
        trinex_always_check(render_pass, "Render pass can't be nullptr!");

        for (GLuint i = 0, size = static_cast<GLuint>(render_pass->color_attachments.size()); i < size; i++)
        {
            auto& attachment = pipeline->color_blending;
            if (attachment.enable)
            {
                new_command(glEnablei, GL_BLEND, i);
                GLenum src_color_func = blend_func(attachment.src_color_func);
                GLenum dst_color_func = blend_func(attachment.dst_color_func);
                GLenum src_alpha_func = blend_func(attachment.src_alpha_func);
                GLenum dst_alpha_func = blend_func(attachment.dst_alpha_func);
                GLenum color_op       = blend_op(attachment.color_op);
                GLenum alpha_op       = blend_op(attachment.alpha_op);

                new_command(glBlendFuncSeparatei, i, src_color_func, dst_color_func, src_alpha_func, dst_alpha_func);
                new_command(glBlendEquationSeparatei, i, color_op, alpha_op);

                EnumerateType mask = enum_value_of(attachment.color_mask);
                GLboolean r_mask   = (mask & enum_value_of(ColorComponent::R)) == enum_value_of(ColorComponent::R);
                GLboolean g_mask   = (mask & enum_value_of(ColorComponent::G)) == enum_value_of(ColorComponent::G);
                GLboolean b_mask   = (mask & enum_value_of(ColorComponent::B)) == enum_value_of(ColorComponent::B);
                GLboolean a_mask   = (mask & enum_value_of(ColorComponent::A)) == enum_value_of(ColorComponent::A);

                new_command(glColorMaski, i, r_mask, g_mask, b_mask, a_mask);
            }
            else
                new_command(glDisablei, static_cast<GLenum>(GL_BLEND), static_cast<GLuint>(i));
        }

        info_log("OpenGL", "Writed %zu commands for pipeline '%s'", m_apply_state.size(), pipeline->full_name().c_str());
    }

    void OpenGL_Pipeline::bind()
    {
        if (OPENGL_API->m_current_pipeline != this)
        {
            OPENGL_API->m_current_pipeline = this;
            glBindProgramPipeline(m_pipeline);
            glBindVertexArray(m_vao);

            for (ExecutableObject* object : m_apply_state)
            {
                object->execute();
            }
        }
    }

    OpenGL_Pipeline::~OpenGL_Pipeline()
    {
        if (m_pipeline)
        {
            glDeleteProgramPipelines(1, &m_pipeline);
        }

        if (m_vao)
        {
            glDeleteVertexArrays(1, &m_vao);
        }

        for (ExecutableObject* object : m_apply_state)
        {
            delete object;
        }
    }

    RHI_Pipeline* OpenGL::create_pipeline(const Pipeline* pipeline)
    {
        OpenGL_Pipeline* opengl_pipeline = new OpenGL_Pipeline();
        opengl_pipeline->init(pipeline);
        return opengl_pipeline;
    }
}// namespace Engine