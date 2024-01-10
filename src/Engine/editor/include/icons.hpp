#pragma once

namespace Engine
{
    namespace ImGuiRenderer
    {
        class ImGuiTexture;
    }

    class Sampler;
    class Texture2D;

    namespace Icons
    {
        Sampler* default_sampler();
        Texture2D* default_texture();
        ImGuiRenderer::ImGuiTexture* find_imgui_icon(class Object* object);
        ImGuiRenderer::ImGuiTexture* find_output_icon();
    }// namespace Icons
}// namespace Engine
