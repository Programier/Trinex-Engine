#pragma once
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>
#include <ScriptEngine/script_object.hpp>

namespace Engine
{
    class EditorViewportClient : public ViewportClient
    {
    public:
        declare_class(EditorViewportClient, ViewportClient);

        class Texture2D* texture                      = nullptr;
        class Sampler* sampler                        = nullptr;
        ImGuiRenderer::ImGuiTexture* _M_imgui_texture = nullptr;
        ScriptObject _M_script_object;

    public:
        EditorViewportClient();

        ViewportClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        ViewportClient& render(class RenderViewport* viewport) override;
        ViewportClient& update(class RenderViewport* viewport, float dt) override;
        ViewportClient& prepare_render(class RenderViewport* viewport) override;

        ViewportClient& destroy_script_object(ScriptObject* object) override;


        EditorViewportClient& create_properties_window(float dt);
        EditorViewportClient& create_scene_tree_window(float dt);
        EditorViewportClient& create_log_window(float dt);
        EditorViewportClient& create_viewport_window(float dt);
        EditorViewportClient& create_bar(float dt);
    };
}// namespace Engine
