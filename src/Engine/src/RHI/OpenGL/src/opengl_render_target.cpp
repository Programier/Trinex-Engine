#include <Core/colors.hpp>
#include <Core/memory.hpp>
#include <Graphics/render_surface.hpp>
#include <opengl_api.hpp>
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

    TreeMap<HashIndex, OpenGL_RenderTarget*> OpenGL_RenderTarget::m_render_targets;

    void OpenGL_RenderTarget::release_all()
    {
        for (auto& rt : m_render_targets)
        {
            rt.second->m_textures.clear();
            delete rt.second;
        }

        m_render_targets.clear();
    }

    OpenGL_RenderTarget* OpenGL_RenderTarget::current()
    {
        return OPENGL_API->m_render_target;
    }

    OpenGL_RenderTarget* OpenGL_RenderTarget::find_or_create(const Span<RenderSurface*>& color_attachments,
                                                             RenderSurface* depth_stencil)
    {
        HashIndex hash = 0;

        for (auto& texture : color_attachments)
        {
            RHI_Object* object = texture->rhi_object<RHI_Object>();
            hash               = memory_hash_fast(&object, sizeof(object), hash);
        }

        if (depth_stencil)
        {
            RHI_Object* object = depth_stencil->rhi_object<RHI_Object>();
            hash               = memory_hash_fast(&object, sizeof(object), hash);
        }

        auto it = m_render_targets.find(hash);
        if (it != m_render_targets.end())
        {
            return it->second;
        }
        OpenGL_RenderTarget* rt = new OpenGL_RenderTarget();
        rt->m_index             = hash;
        m_render_targets[hash]  = rt;

        rt->init(color_attachments, depth_stencil);
        return rt;
    }

    OpenGL_RenderTarget* OpenGL_RenderTarget::find_or_create(const Span<OpenGL_Texture*>& color_attachments,
                                                             OpenGL_Texture* depth_stencil)
    {

        HashIndex hash = 0;
        for (RHI_Object* texture : color_attachments)
        {
            hash = memory_hash_fast(&texture, sizeof(texture), hash);
        }

        if (depth_stencil)
        {
            hash = memory_hash_fast(&depth_stencil, sizeof(depth_stencil), hash);
        }


        auto it = m_render_targets.find(hash);
        if (it != m_render_targets.end())
        {
            return it->second;
        }
        OpenGL_RenderTarget* rt = new OpenGL_RenderTarget();
        rt->m_index             = hash;
        m_render_targets[hash]  = rt;

        rt->init(color_attachments, depth_stencil);
        return rt;
    }

    void OpenGL_RenderTarget::bind()
    {
        OPENGL_API->m_render_target = this;
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    }

    template<typename T>
    static GLuint create_framebuffer(OpenGL_RenderTarget* self, const Span<T*>& color_attachments, T* depth_stencil,
                                     OpenGL_Texture* (*callback)(T*) )
    {
        GLuint m_framebuffer = 0;
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        size_t index = 0;

        Vector<GLenum> color_attachment_indices;
        color_attachment_indices.reserve(color_attachments.size());

        for (const auto& color_attachment : color_attachments)
        {
            auto color_texture = color_attachment;
            info_log("Framebuffer", "Attaching texture[%p] to buffer %p", color_texture, self);
            self->attach_texture(callback(color_texture), GL_COLOR_ATTACHMENT0 + index);
            color_attachment_indices.push_back(GL_COLOR_ATTACHMENT0 + index);
            index++;
        }

        if (depth_stencil)
        {
            self->attach_texture(callback(depth_stencil), get_attachment_type(callback(depth_stencil)->m_format.m_format));
        }

        if (color_attachment_indices.size() > 0)
        {
            glDrawBuffers(color_attachment_indices.size(), color_attachment_indices.data());
        }
        return m_framebuffer;
    }

    OpenGL_RenderTarget& OpenGL_RenderTarget::init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
    {
        m_framebuffer = create_framebuffer<RenderSurface>(
                this, color_attachments, depth_stencil,
                [](RenderSurface* texture) -> OpenGL_Texture* { return texture->rhi_object<OpenGL_Texture>(); });
        return *this;
    }

    OpenGL_RenderTarget& OpenGL_RenderTarget::init(const Span<struct OpenGL_Texture*>& color_attachments,
                                                   struct OpenGL_Texture* depth_stencil)
    {
        m_framebuffer = create_framebuffer<OpenGL_Texture>(this, color_attachments, depth_stencil,
                                                           [](OpenGL_Texture* texture) -> OpenGL_Texture* { return texture; });
        return *this;
    }

    OpenGL_RenderTarget& OpenGL_RenderTarget::attach_texture(const OpenGL_Texture* texture, GLuint attachment)
    {
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

        m_textures.push_back(texture);
        texture->m_surface_state->m_render_targets.insert(this);
        return *this;
    }

    OpenGL_RenderTarget::~OpenGL_RenderTarget()
    {
        if (m_framebuffer != 0)
        {
            glDeleteFramebuffers(1, &m_framebuffer);
        }

        for (auto& surface : m_textures)
        {
            surface->m_surface_state->m_render_targets.erase(this);
        }

        m_render_targets.erase(m_index);
    }


    OpenGL_RenderTarget::Saver::Saver() : m_viewport(OPENGL_API->viewport()), m_rt(OpenGL_RenderTarget::current())
    {}

    OpenGL_RenderTarget::Saver::~Saver()
    {
        if (m_rt)
        {
            m_rt->bind();
        }

        OPENGL_API->viewport(m_viewport);
    }

    void OpenGL::bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
    {
        auto rt = OpenGL_RenderTarget::find_or_create(color_attachments, depth_stencil);
        rt->bind();
    }

    void OpenGL::bind_render_target(const Span<struct OpenGL_Texture*>& color_attachments, struct OpenGL_Texture* depth_stencil)
    {
        auto rt = OpenGL_RenderTarget::find_or_create(color_attachments, depth_stencil);
        rt->bind();
    }

    void OpenGL::viewport(const ViewPort& viewport)
    {
        bool changed = false;

        if (glm::any(glm::epsilonNotEqual(viewport.pos, m_viewport.pos, Point2D(0.001f, 0.001f))) ||
            glm::any(glm::epsilonNotEqual(viewport.size, m_viewport.size, Size2D(0.001f, 0.001f))))
        {
            glViewport(viewport.pos.x, viewport.pos.y, viewport.size.x, viewport.size.y);
            changed = true;
        }

        if (glm::epsilonNotEqual(viewport.min_depth, m_viewport.min_depth, 0.001f) ||
            glm::epsilonNotEqual(viewport.max_depth, m_viewport.max_depth, 0.001f))
        {
            glDepthRangef(viewport.min_depth, viewport.max_depth);
            changed = true;
        }

        if (changed)
        {
            m_viewport = viewport;
        }
    }

    ViewPort OpenGL::viewport()
    {
        return m_viewport;
    }
}// namespace Engine
