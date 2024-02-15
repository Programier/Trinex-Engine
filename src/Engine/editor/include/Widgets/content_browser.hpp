#pragma once
#include <Core/callback.hpp>
#include <Graphics/imgui.hpp>

namespace Engine
{
    class ContentBrowser : public ImGuiRenderer::ImGuiAdditionalWindow
    {
    private:
        enum class PackageNodeType
        {
            Folder,
            Package,
            NotLoadedPackage
        };


        struct PackageTreeNode {
            Path _M_path;
            String _M_name;
            Package* _M_package = nullptr;
            bool _M_is_builded  = false;
            bool _M_is_package  = false;

            TreeMap<String, PackageTreeNode*> _M_childs;
            void clean();
            void rebuild();
            PackageNodeType type() const;
            ~PackageTreeNode();
        };


        bool loaded_package_popup(void* data);
        bool not_loaded_package_popup(void* data);
        bool folder_package_popup(void* data);
        bool render_package_popup(void* data);
        void render_package_tree(PackageTreeNode* node);
        void render_package_popup();
        void render_packages();
        bool show_context_menu(void* userdata);
        void render_content_window();

        void create_dock_space();

        PackageTreeNode* _M_show_popup_for   = nullptr;
        PackageTreeNode* _M_root             = nullptr;
        PackageTreeNode* _M_selected_package = nullptr;

        bool _M_show_context_menu = false;

        ImGuiID _M_dock_window_id;

    public:
        CallBacks<void(Object*)> on_object_select;
        CallBacks<bool(class Class*)> filters;

        class Object* selected_object = nullptr;

        void init(RenderViewport* viewport) override;
        bool render(RenderViewport* viewport) override;
        Package* selected_package() const;

        static const char* name();
        ~ContentBrowser();
    };
}// namespace Engine
