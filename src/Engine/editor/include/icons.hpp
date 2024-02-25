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
        enum IconType
        {
            Default,
            Add,
            Remove,
            Select,
            Move,
            Rotate,
            Scale,
            __COUNT__
        };

        ImGuiRenderer::ImGuiTexture* icon(IconType type);
        Sampler* default_sampler();
        Texture2D* default_texture();

        ImGuiRenderer::ImGuiTexture* find_imgui_icon(class Object* object);
    }// namespace Icons
}// namespace Engine
