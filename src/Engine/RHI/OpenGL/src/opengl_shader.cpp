#include <algorithm>
#include <opengl_api.hpp>
#include <opengl_shader.hpp>
#include <opengl_types.hpp>


#define DESTROY_CALL(func, id)                                                                                         \
    if (id)                                                                                                            \
    func(id)


namespace Engine
{

    implement_opengl_instance_cpp(OpenGL_Shader);

    static GLint compile_shader_module(const FileBuffer& shader_code, GLenum type, const String& name)
    {
        GLint ID = 0;

        if (shader_code.empty())
            return ID;

        static GLchar log[1024];
        ID = glCreateShader(type);

        const GLchar* code = reinterpret_cast<const GLchar*>(shader_code.data());

        glShaderSource(ID, 1, &code, nullptr);
        glCompileShader(ID);

        GLint succes;
        glGetShaderiv(ID, GL_COMPILE_STATUS, &succes);
        if (!succes)
        {
            glGetShaderInfoLog(ID, 1024, nullptr, log);
            opengl_debug_log("Failed to compile %s shader '%s'\n\n%s\n", shader_type_name(type).c_str(), name.c_str(),
                             log);
            glDeleteShader(ID);
            ID = 0;
        }

        return ID;
    }


    OpenGL_Shader::~OpenGL_Shader()
    {
        DESTROY_CALL(glDeleteShader, _M_vertex);
        DESTROY_CALL(glDeleteShader, _M_fragment);
        DESTROY_CALL(glDeleteProgram, _M_instance_id);
    }

    template<typename... Args>
    static void attach_shader(GLint instance, Args... args)
    {
        for (auto& ell : {args...})
        {
            glAttachShader(instance, static_cast<GLuint>(ell));
        }
    }

#define new_shader_command(...) shader->_M_command_buffer.next(new OpenGL_Command(__VA_ARGS__))

    static void create_command_buffer(OpenGL_Shader* shader, const PipelineState* state, const String& shader_name)
    {
        // Depth test
        if (state->depth_test.enable)
        {
            new_shader_command(glEnable, static_cast<GLenum>(GL_DEPTH_TEST));
            new_shader_command(glDepthMask, state->depth_test.write_enable);
            new_shader_command(glDepthFunc, get_type(state->depth_test.func));
        }
        else
            new_shader_command(glDisable, static_cast<GLenum>(GL_DEPTH_TEST));

        // Stencil test
        if (state->stencil_test.enable)
        {
            new_shader_command(glEnable, static_cast<GLenum>(GL_STENCIL_TEST));

            static GLenum faces[2]                                         = {GL_FRONT, GL_BACK};
            const PipelineState::StencilTestInfo::FaceInfo* state_faces[2] = {&state->stencil_test.front,
                                                                              &state->stencil_test.back};

            for (int i = 0; i < 2; i++)
            {
                GLenum sfail  = static_cast<GLenum>(get_type(state_faces[i]->fail));
                GLenum dpfail = static_cast<GLenum>(get_type(state_faces[i]->depth_fail));
                GLenum dppass = static_cast<GLenum>(get_type(state_faces[i]->depth_pass));

                shader->_M_command_buffer.next(
                        new OpenGL_Command(glStencilOpSeparate, faces[i], sfail, dpfail, dppass));


                new_shader_command(glStencilMaskSeparate, faces[i], static_cast<GLuint>(state_faces[i]->write_mask));

                new_shader_command(glStencilFuncSeparate, faces[i],
                                   static_cast<GLenum>(get_type(state_faces[i]->compare)),
                                   static_cast<GLint>(state_faces[i]->reference),
                                   static_cast<GLuint>(state_faces[i]->compare_mask));
            }
        }
        else
            new_shader_command(glDisable, static_cast<GLenum>(GL_STENCIL_TEST));

        // Input assembly
        if (state->input_assembly.primitive_restart_enable)
            new_shader_command(glEnable, static_cast<GLenum>(GL_PRIMITIVE_RESTART_FIXED_INDEX));
        else
            new_shader_command(glDisable, static_cast<GLenum>(GL_PRIMITIVE_RESTART_FIXED_INDEX));

        shader->_M_topology = static_cast<GLenum>(get_type(state->input_assembly.primitive_topology));

        // Rasterizer settings
#if GL_DEPTH_CLAMP_EXT
        if (state->rasterizer.depth_clamp_enable)
        {
            new_shader_command(glEnable, static_cast<GLenum>(GL_DEPTH_CLAMP_EXT));
        }
        else
#endif
        {
            new_shader_command(glDisable, static_cast<GLenum>(GL_POLYGON_OFFSET_FILL));
        }

        if (state->rasterizer.depth_bias_enable)
        {
            new_shader_command(glEnable, static_cast<GLenum>(GL_POLYGON_OFFSET_FILL));
            new_shader_command(glPolygonOffset, state->rasterizer.depth_bias_const_factor,
                               state->rasterizer.depth_bias_slope_factor);
        }
        else
        {
            new_shader_command(glDisable, static_cast<GLenum>(GL_POLYGON_OFFSET_FILL));
        }

        if (state->rasterizer.poligon_mode != PolygonMode::Fill)
        {
            opengl_error("OpenGL", "OpenGL support only PolygonMode::Fill");
        }

        new_shader_command(
                glFrontFace,
                static_cast<GLenum>(state->rasterizer.front_face == FrontFace::CounterClockWise ? GL_CCW : GL_CW));

        if (state->rasterizer.cull_mode != CullMode::None)
        {
            new_shader_command(glDisable, static_cast<GLenum>(GL_CULL_FACE));
            GLenum cull_mode = get_type(state->rasterizer.cull_mode);
            new_shader_command(glCullFace, cull_mode);
        }
        else
        {
            new_shader_command(glDisable, static_cast<GLenum>(GL_CULL_FACE));
        }
        new_shader_command(glLineWidth, state->rasterizer.line_width);


        // Blending
        if (state->color_blending.logic_op_enable)
        {
            opengl_error("OpenGL", "Logic op is not supported");
        }

        new_shader_command(glBlendColor, state->color_blending.blend_constants.vector.r,
                           state->color_blending.blend_constants.vector.g,
                           state->color_blending.blend_constants.vector.b,
                           state->color_blending.blend_constants.vector.a);


        GLuint size = static_cast<GLuint>(state->color_blending.blend_attachment.size());

        for (GLuint i = 0; i < size; i++)
        {
            auto& attachment = state->color_blending.blend_attachment[i];
            if (attachment.enable)
            {
                new_shader_command(glEnablei, static_cast<GLenum>(GL_BLEND), i);
                GLenum src_color_func = static_cast<GLenum>(get_type(attachment.src_color_func));
                GLenum dst_color_func = static_cast<GLenum>(get_type(attachment.dst_color_func));
                GLenum src_alpha_func = static_cast<GLenum>(get_type(attachment.src_alpha_func));
                GLenum dst_alpha_func = static_cast<GLenum>(get_type(attachment.dst_alpha_func));
                GLenum color_op       = static_cast<GLenum>(get_type(attachment.color_op));
                GLenum alpha_op       = static_cast<GLenum>(get_type(attachment.alpha_op));

                new_shader_command(glBlendFuncSeparatei, i, src_color_func, dst_color_func, src_alpha_func,
                                   dst_alpha_func);

                new_shader_command(glBlendEquationSeparatei, i, color_op, alpha_op);

                GLboolean r_mask = (attachment.color_mask & mask_of<ColorComponentMask>(ColorComponent::R)) ==
                                   mask_of<ColorComponentMask>(ColorComponent::R);
                GLboolean g_mask = (attachment.color_mask & mask_of<ColorComponentMask>(ColorComponent::G)) ==
                                   mask_of<ColorComponentMask>(ColorComponent::G);
                GLboolean b_mask = (attachment.color_mask & mask_of<ColorComponentMask>(ColorComponent::B)) ==
                                   mask_of<ColorComponentMask>(ColorComponent::B);
                GLboolean a_mask = (attachment.color_mask & mask_of<ColorComponentMask>(ColorComponent::A)) ==
                                   mask_of<ColorComponentMask>(ColorComponent::A);

                new_shader_command(glColorMaski, i, r_mask, g_mask, b_mask, a_mask);
            }
            else
                new_shader_command(glDisablei, static_cast<GLenum>(GL_BLEND), static_cast<GLuint>(i));
        }

        opengl_debug_log("OpenGL", "Shader '%s' has command buffer with %zu commands", shader_name.data(),
                         shader->_M_command_buffer.length());
    }

    OpenGL& OpenGL::create_shader(Identifier& ID, const PipelineCreateInfo& info)
    {
        OpenGL_Shader* shader = new OpenGL_Shader();
        ID                    = 0;

        if ((shader->_M_vertex = compile_shader_module(info.text.vertex, GL_VERTEX_SHADER, info.name)) == 0)
        {
            delete shader;
            shader = nullptr;
        }

        if (!shader ||
            (shader->_M_fragment = compile_shader_module(info.text.fragment, GL_FRAGMENT_SHADER, info.name)) == 0)
        {
            delete shader;
            shader = nullptr;
        }

        if (shader == nullptr)
            return *this;

        static GLchar log[1024];
        shader->_M_instance_id = glCreateProgram();
        attach_shader(shader->_M_instance_id, shader->_M_vertex, shader->_M_fragment);
        glLinkProgram(shader->_M_instance_id);

        GLint succes;
        glGetProgramiv(shader->_M_instance_id, GL_LINK_STATUS, &succes);
        if (!succes)
        {
            glGetProgramInfoLog(shader->_M_instance_id, 1024, nullptr, log);
            opengl_debug_log("Shader: Failed to link shader program '%s'\n\n%s\n", info.name.c_str(), log);
            delete shader;
        }

        if (shader)
        {
            ID = shader->ID();

            shader->_M_attributes = info.vertex_info.attributes;
            std::sort(shader->_M_attributes.begin(), shader->_M_attributes.end(),
                      [](VertexAttribute a, VertexAttribute b) -> bool { return a.offset < b.offset; });
            shader->_M_vertex_size = static_cast<uint_t>(info.vertex_info.size);

            for (auto& ubo : info.uniform_buffers)
            {
                GLuint uniform_block_index = glGetUniformBlockIndex(shader->_M_instance_id, ubo.name.c_str());
                shader->_M_block_indices.insert({ubo.binding, uniform_block_index});
            }

            create_command_buffer(shader, &info.state, info.name);
        }
        return *this;
    }

    OpenGL& OpenGL::use_shader(const Identifier& ID)
    {
        if (ID)
        {
            state.shader = GET_TYPE(OpenGL_Shader, ID);
            glUseProgram(state.shader->_M_instance_id);
            state.shader->_M_command_buffer.apply();
        }
        else
            glUseProgram(0);

        return *this;
    }

    OpenGL& OpenGL::create_ssbo(Identifier&, const byte* data, size_t size)
    {
        throw std::runtime_error(not_implemented);
    }

    OpenGL& OpenGL::bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size)
    {
        throw std::runtime_error(not_implemented);
    }

    OpenGL& OpenGL::update_ssbo(const Identifier&, const byte*, size_t offset, size_t size)
    {
        throw std::runtime_error(not_implemented);
    }

    OpenGL_Shader& OpenGL_Shader::apply_vertex_attributes(ArrayOffset base_offset)
    {
        int index = 0;

        static const Set<GLuint> _M_attrib_types = {
                GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FLOAT,
        };

        for (const auto& value : _M_attributes)
        {

            auto type = _M_shader_types.at(value.type.type);

            if (_M_attrib_types.contains(type.type))
            {
                glVertexAttribPointer(index, type.size, type.type, GL_FALSE, _M_vertex_size,
                                      (GLvoid*) (value.offset + base_offset));
            }
            else
            {
                glVertexAttribIPointer(index, type.size, type.type, _M_vertex_size,
                                       (GLvoid*) (value.offset + base_offset));
            }

            glEnableVertexAttribArray(index++);
        }

        return *this;
    }


}// namespace Engine
