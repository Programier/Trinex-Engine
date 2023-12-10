#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
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


    Index OpenGL_RenderTarget::bind()
    {
        if (OPENGL_API->_M_current_render_target != this)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _M_framebuffer);
            OPENGL_API->_M_current_render_target = this;

            update_viewport();
            update_scissors();

            _M_render_pass->apply(this);
        }

        return 0;
    }

    void OpenGL_RenderTarget::viewport(const ViewPort& viewport)
    {
        _M_viewport = viewport;
        if (is_active())
        {
            update_viewport();
        }
    }

    void OpenGL_RenderTarget::scissor(const Scissor& scissor)
    {
        _M_scissor = scissor;
        if (is_active())
        {
            update_scissors();
        }
    }

    void OpenGL_RenderTarget::clear_depth_stencil(const DepthStencilClearValue& value)
    {}

    void OpenGL_RenderTarget::clear_color(const ColorClearValue& color, byte layout)
    {}

    bool OpenGL_RenderTarget::is_active() const
    {
        return OPENGL_API->_M_current_render_target == this;
    }

    void OpenGL_RenderTarget::update_viewport()
    {
        glViewport(_M_viewport.pos.x, _M_viewport.pos.y, _M_viewport.size.x, _M_viewport.size.y);
#if USING_OPENGL_CORE
        glDepthRange(_M_viewport.min_depth, _M_viewport.max_depth);
#else
        glDepthRangef(_M_viewport.min_depth, _M_viewport.max_depth);
#endif
    }

    void OpenGL_RenderTarget::update_scissors()
    {
        glScissor(_M_scissor.pos.x, _M_scissor.pos.y, _M_scissor.size.x, _M_scissor.size.y);
    }


    OpenGL_MainRenderTarget::OpenGL_MainRenderTarget()
    {
        _M_render_pass = OPENGL_API->_M_main_render_pass;
        _M_clear_color.push_back(Colors::Black);
    }

    bool OpenGL_MainRenderTarget::is_destroyable() const
    {
        return false;
    }

    OpenGL_MainRenderTarget::~OpenGL_MainRenderTarget()
    {
        _M_render_pass = nullptr;
    }

    OpenGL_RenderTarget& OpenGL_RenderTarget::init(const RenderTarget* render_target)
    {
        glGenFramebuffers(1, &_M_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, _M_framebuffer);

        _M_render_pass = render_target->render_pass->rhi_object<OpenGL_RenderPass>();

        size_t index           = 0;
        _M_clear_color         = render_target->color_clear;
        _M_depth_stencil_clear = render_target->depth_stencil_clear;

        _M_viewport.size      = render_target->size;
        _M_viewport.pos       = {0.0f, 0.0f};
        _M_viewport.min_depth = 0.0f;
        _M_viewport.max_depth = 1.0f;

        _M_scissor.size = render_target->size;
        _M_scissor.pos  = {0.0f, 0.0f};

        for (const Texture2D* color_attachment : render_target->frame(0)->color_attachments)
        {
            info_log("Framebuffer", "Attaching texture[%p] to buffer %p", color_attachment, this);
            attach_texture(color_attachment, GL_COLOR_ATTACHMENT0 + index);
            index++;
        }

        if (_M_render_pass->_M_has_depth_stencil_attachment)
        {
            const Texture2D* depth_attachment = render_target->frame(0)->depth_stencil_attachment;
            attach_texture(depth_attachment,
                           get_attachment_type(depth_attachment->rhi_object<OpenGL_Texture>()->_M_format._M_format));
        }

        if (OPENGL_API->_M_current_render_target != nullptr)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, OPENGL_API->_M_current_render_target->_M_framebuffer);
        }

        return *this;
    }

    OpenGL_RenderTarget& OpenGL_RenderTarget::attach_texture(const Texture2D* texture_attachment, GLuint attachment)
    {
        OpenGL_Texture* texture = texture_attachment->rhi_object<OpenGL_Texture>();

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture->_M_type, texture->_M_id, 0);

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
            error_log("Framebuffer",
                      "Combination of internal formats used by attachments in thef ramebuffer results in a "
                      "nonrednerable target");
        }
        else
        {
            info_log("Framebuffer", "Attach success!\n");
        }
        return *this;
    }

    RHI_RenderTarget* OpenGL::create_render_target(const RenderTarget* target)
    {
        if(target->frames_count() != render_target_buffer_count())
            throw EngineException("Frames count is mismatch with API requirements");
        return &((new OpenGL_RenderTarget())->init(target));
    }
}// namespace Engine
