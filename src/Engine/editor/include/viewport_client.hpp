#pragma once
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>

namespace Engine
{
    class EditorViewportClient : public ViewportClient
    {
    public:
        declare_class(EditorViewportClient, ViewportClient);

        class Texture2D* texture                      = nullptr;
        class Sampler* sampler                        = nullptr;
        ImGuiRenderer::ImGuiTexture* _M_imgui_texture = nullptr;

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
