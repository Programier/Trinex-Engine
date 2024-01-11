#pragma once
#include <Core/callback.hpp>
#include <Graphics/imgui.hpp>

namespace Engine
{
    class ImGuiNotificationMessage : public ImGuiRenderer::ImGuiAdditionalWindow
    {
    public:
        enum Type
        {
            Info,
            Warning,
            Error
        };

    private:
        String _M_message;
        Type _M_type;

    public:
        ImGuiNotificationMessage(const String& msg, Type type = Type::Info);
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiCreateNewPackage : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        class Package* _M_parent = nullptr;
        String new_package_name;
        bool allow_rename = false;

    public:
        ImGuiCreateNewPackage(class Package* pkg);
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiCreateNewAsset : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        class Package* _M_parent = nullptr;
        String new_asset_name;
        bool allow_rename = false;
        int current_index = 0;

        Vector<class Class*> _M_filtered_classes;

    public:
        CallBacks<bool(class Class*)> filters;

        ImGuiCreateNewAsset(class Package* pkg, const CallBacks<bool(class Class*)>& = {});
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiRenameObject : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        class Object* _M_object = nullptr;
        String new_object_name;
        bool allow_rename = false;

    public:
        ImGuiRenameObject(class Object* object);
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiPackageTree : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        class Package* _M_selected = nullptr;
        bool _M_open_package_popup = false;

        void render_internal(Package* package);
        bool render_popup_internal(void* userdata);
        void render_popup(RenderViewport* viewport);

    public:
        CallBacks<void(Package*)> on_package_select;

        ImGuiPackageTree();
        bool render(class RenderViewport* viewport) override;
        Package* selected_package() const;
        static const char* name();
    };

    class ImGuiContentBrowser : public ImGuiRenderer::ImGuiAdditionalWindow
    {
    private:
        bool _M_show_context_menu = false;
        bool show_context_menu(void* userdata);

    public:
        Object* selected = nullptr;
        Package* package = nullptr;
        CallBacks<void(Object*)> on_object_selected;
        CallBacks<bool(Class*)> filters;

        bool render(RenderViewport* viewport) override;
        static const char* name();
    };


    class ImGuiOpenFile : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        Package* _M_package = nullptr;
        Function<void(Package*, const Path&)> _M_callback;
        void* _M_browser = nullptr;

    public:
        ImGuiOpenFile(Package* pkg, const Function<void(Package*, const Path&)>& callback,
                      const Vector<String>& type_filters = {});
        bool render(RenderViewport* viewport) override;
        static const char* name();

        ~ImGuiOpenFile();
    };

    class ImGuiObjectProperties : public ImGuiRenderer::ImGuiAdditionalWindow
    {

    public:
        Object* object = nullptr;

        bool render(RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiSceneTree : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        void render_scene_tree(class SceneComponent* component);

    public:
        class SceneComponent* root_component = nullptr;
        class SceneComponent* selected       = nullptr;

        CallBacks<void(SceneComponent*)> on_node_select;

        ImGuiSceneTree(SceneComponent* root_component = nullptr);
        bool render(RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiCreateNode : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        class VisualMaterial* _M_material;
        Map<String, Vector<class Struct*>>* _M_nodes;
        class Struct* _M_selected = nullptr;
        int _M_current_index      = 0;

    public:
        ImGuiCreateNode(class VisualMaterial* material);

        bool render(RenderViewport* viewport) override;
        static const char* name();
    };
}// namespace Engine
