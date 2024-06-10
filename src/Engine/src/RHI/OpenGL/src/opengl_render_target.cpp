#include <Core/colors.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/render_target_texture.hpp>
#include <opengl_api.hpp>
#include <opengl_render_pass.hpp>
#include <opengl_render_target.hpp>
#include <opengl_texture.hpp>

namespace Engine
{
    static GLenum get_attachment_type(GLenum type)
    {
        switch (type)
        {
            case GL_DEPTH_STENCIL:
                return GL_DEPTH_STENCIL_ATTACHMENT;
            case GL_DEPTH_COMPONENT:
                return GL_DEPTH_ATTACHMENT;
            case GL_STENCIL_INDEX:
                return GL_STENCIL_ATTACHMENT;
            default:
                return GL_COLOR_ATTACHMENT0;
        }
    }

    void OpenGL_RenderTarget::bind()
    {
        if (OPENGL_API->m_current_render_target != this)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
            OPENGL_API->m_current_render_target = this;
            update_viewport();
            update_scissors();
        }
    }

    void OpenGL_RenderTarget::viewport(const ViewPort& viewport)
    {
        m_viewport = viewport;
        if (is_active())
        {
            update_viewport();
        }
    }

    void OpenGL_RenderTarget::scissor(const Scissor& scissor)
    {
        m_scissor = scissor;
        if (is_active())
        {
            update_scissors();
        }
    }

    void OpenGL_RenderTarget::clear_depth_stencil(const DepthStencilClearValue& value)
    {
        auto current = OPENGL_API->m_current_render_target;

        bind();

        glDepthMask(GL_TRUE);
        glStencilMask(0xFFFFFFFF);
        glClearBufferfi(GL_DEPTH_STENCIL, 0, value.depth, value.stencil);

        if (current)
        {
            current->bind();
        }
    }

    void OpenGL_RenderTarget::clear_color(const ColorClearValue& color, byte layout)
    {
        auto current = OPENGL_API->m_current_render_target;

        bind();

        glClearBufferfv(GL_COLOR, static_cast<GLint>(layout), &color.x);

        if (current)
        {
            current->bind();
        }
    }

    bool OpenGL_RenderTarget::is_active() const
    {
        return OPENGL_API->m_current_render_target == this;
    }

    void OpenGL_RenderTarget::update_viewport()
    {
        glViewport(m_viewport.pos.x, m_viewport.pos.y, m_viewport.size.x, m_viewport.size.y);
#if USING_OPENGL_CORE
        glDepthRange(m_viewport.min_depth, m_viewport.max_depth);
#else
        glDepthRangef(m_viewport.min_depth, m_viewport.max_depth);
#endif
    }

    void OpenGL_RenderTarget::update_scissors()
    {
        glScissor(m_scissor.pos.x, m_scissor.pos.y, m_scissor.size.x, m_scissor.size.y);
    }

    OpenGL_RenderTarget& OpenGL_RenderTarget::init(const RenderTarget* render_target)
    {
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        size_t index               = 0;
        m_has_depth_stencil_buffer = render_target->render_pass->depth_stencil_attachment != ColorFormat::Undefined;

        m_viewport.size      = render_target->size;
        m_viewport.pos       = {0.0f, 0.0f};
        m_viewport.min_depth = 0.0f;
        m_viewport.max_depth = 1.0f;

        m_scissor.size = render_target->size;
        m_scissor.pos  = {0.0f, 0.0f};

        Vector<GLenum> color_attachments;
        color_attachments.reserve(render_target->color_attachments.size());

        for (const auto& color_attachment : render_target->color_attachments)
        {
            auto color_texture = color_attachment.ptr();
            info_log("Framebuffer", "Attaching texture[%p] to buffer %p", color_texture, this);
            attach_texture(color_texture, GL_COLOR_ATTACHMENT0 + index);
            color_attachments.push_back(GL_COLOR_ATTACHMENT0 + index);
            index++;
        }

        auto* depth_attachment = render_target->depth_stencil_attachment.ptr();

        if (depth_attachment)
        {
            attach_texture(depth_attachment,
                           get_attachment_type(depth_attachment->rhi_object<OpenGL_Texture>()->m_format.m_format));
        }

        glDrawBuffers(color_attachments.size(), color_attachments.data());


        if (OPENGL_API->m_current_render_target != nullptr)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, OPENGL_API->m_current_render_target->m_framebuffer);
        }

        return *this;
    }

    OpenGL_RenderTarget& OpenGL_RenderTarget::attach_texture(const Texture2D* texture_attachment, GLuint attachment)
    {
        OpenGL_Texture* texture = texture_attachment->rhi_object<OpenGL_Texture>();

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture->m_type, texture->m_id, 0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
        {
            error_log("Framebuffer", "Incomplete framebuffer attachments\n");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
        {
            error_log("Framebuffer", "incomplete missing framebuffer attachments");
        }
#if USING_OPENGL_CORE
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
        {
            error_log("Framebuffer", "Incomplete framebuffer attachments dimensions\n");
        }
#endif
        else if (status == GL_FRAMEBUFFER_UNSUPPORTED)
        {
            error_log("Framebuffer", "Combination of internal formats used by attachments in thef ramebuffer results in a "
                                     "nonrednerable target");
        }
        else
        {
            info_log("Framebuffer", "Attach success!\n");
        }
        return *this;
    }

    OpenGL_RenderTarget::~OpenGL_RenderTarget()
    {
        if (m_framebuffer != 0)
        {
            glDeleteFramebuffers(1, &m_framebuffer);
        }
    }

    OpenGL_MainRenderTarget::OpenGL_MainRenderTarget()
    {}

    void OpenGL_MainRenderTarget::bind()
    {
        glDisable(GL_DEPTH_TEST);
        OpenGL_RenderTarget::bind();
    }

    OpenGL_MainRenderTarget::~OpenGL_MainRenderTarget()
    {}

    RHI_RenderTarget* OpenGL::create_render_target(const RenderTarget* target)
    {
        return &((new OpenGL_RenderTarget())->init(target));
    }
}// namespace Engine
