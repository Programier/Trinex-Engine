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
            Path m_path;
            String m_name;
            Package* m_package = nullptr;
            bool m_is_builded  = false;
            bool m_is_package  = false;

            TreeMap<String, PackageTreeNode*> m_childs;
            void clean();
            void rebuild();
            PackageNodeType type() const;
            PackageTreeNode* find(const Path& path);
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
        void rebuild_package_tree(const Path& selected);

        PackageTreeNode* m_show_popup_for   = nullptr;
        PackageTreeNode* m_root             = nullptr;
        PackageTreeNode* m_selected_package = nullptr;

        bool m_show_context_menu = false;

        ImGuiID m_dock_window_id;


    public:
        CallBacks<void(Object*)> on_object_select;
        CallBacks<void(Object*)> on_object_double_click;
        CallBacks<bool(class Class*)> filters;

        class Object* selected_object = nullptr;

        void init(RenderViewport* viewport) override;
        bool render(RenderViewport* viewport) override;
        Package* selected_package() const;

        static const char* name();
        ~ContentBrowser();
    };
}// namespace Engine
