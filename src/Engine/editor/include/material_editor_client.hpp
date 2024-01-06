#pragma once
#include <Graphics/render_viewport.hpp>
#include <imgui_windows.hpp>

namespace Engine
{
    class MaterialEditorClient : public ViewportClient
    {
        declare_class(MaterialEditorClient, ViewportClient);

    private:
        ImGuiPackageTree _M_package_tree;
        ImGuiContentBrowser _M_content_browser;

        class RenderViewport* _M_viewport            = nullptr;
        class MaterialInterface* _M_current_material = nullptr;
        bool _M_open_package_popup                   = false;
        size_t _M_frame                              = 0;

    public:
        MaterialEditorClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        MaterialEditorClient& render(class RenderViewport* viewport) override;
        MaterialEditorClient& update(class RenderViewport* viewport, float dt) override;
        MaterialEditorClient& prepare_render(class RenderViewport* viewport) override;


        MaterialEditorClient& render_viewport(float dt);
        MaterialEditorClient& render_properties(float dt);

        void on_package_select(Package* package);

        void render_dock_window(void* userdata);
    };
}// namespace Engine
