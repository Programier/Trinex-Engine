#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>
#include <opengl_imgui_texture.hpp>
#include <opengl_api.hpp>
#include <opengl_sampler.hpp>
#include <opengl_texture.hpp>
#include <imgui.h>

namespace Engine
{
    GLuint OpenGL_ImGuiTexture::texture_id()
    {
        if (_M_texture)
        {
            OpenGL_Texture* texture = _M_texture->rhi_object<OpenGL_Texture>();
            if (texture)
            {
                return texture->_M_id;
            }
        }

        return 0;
    }

    GLuint OpenGL_ImGuiTexture::sampler_id()
    {
        if (_M_sampler)
        {
            OpenGL_Sampler* sampler = _M_sampler->rhi_object<OpenGL_Sampler>();
            if (sampler)
            {
                return sampler->_M_id;
            }
        }
        return 0;
    }

    RHI_ImGuiTexture* OpenGL::imgui_create_texture(ImGuiContext* ctx, Texture* texture, Sampler* sampler)
    {
        ImGui::SetCurrentContext(ctx);
        return new OpenGL_ImGuiTexture(texture, sampler);
    }
}// namespace Engine
