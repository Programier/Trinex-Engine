#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>
#include <imgui.h>
#include <opengl_api.hpp>
#include <opengl_imgui_texture.hpp>
#include <opengl_sampler.hpp>
#include <opengl_texture.hpp>

namespace Engine
{
    GLuint OpenGL_ImGuiTexture::texture_id()
    {
        if (m_texture)
        {
            OpenGL_Texture* texture = m_texture->rhi_object<OpenGL_Texture>();
            if (texture)
            {
                return texture->m_id;
            }
        }

        return 0;
    }

    RHI_ImGuiTexture* OpenGL::imgui_create_texture(ImGuiContext* ctx, Texture* texture)
    {
        ImGui::SetCurrentContext(ctx);
        return new OpenGL_ImGuiTexture(texture);
    }
}// namespace Engine
