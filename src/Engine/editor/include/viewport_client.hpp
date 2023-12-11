#pragma once
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
    class EditorViewportClient : public ViewportClient
    {
    public:
        declare_class(EditorViewportClient, ViewportClient);

        class Texture2D* texture = nullptr;
        class Sampler* sampler   = nullptr;
        ImGuiTexture _M_imgui_texture;

    public:
        EditorViewportClient();

        ViewportClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        ViewportClient& render(class RenderViewport* viewport) override;
        ViewportClient& update(class RenderViewport* viewport, float dt) override;
        ViewportClient& prepare_render(class RenderViewport* viewport) override;


        EditorViewportClient& create_docking_window();
        EditorViewportClient& create_properties_window();
        EditorViewportClient& create_scene_tree_window();
        EditorViewportClient& create_log_window();
        EditorViewportClient& create_viewport_window();
        EditorViewportClient& create_bar();
    };
}// namespace Engine
