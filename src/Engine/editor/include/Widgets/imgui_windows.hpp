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
        String m_message;
        Type m_type;

    public:
        ImGuiNotificationMessage(const String& msg, Type type = Type::Info);
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiCreateNewPackage : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        class Package* m_parent = nullptr;
        String new_package_name;
        bool allow_rename = false;
        CallBack<void(Package*)> m_on_create;

    public:
        ImGuiCreateNewPackage(class Package* pkg, const CallBack<void(Package*)>& on_create = {});
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiCreateNewAsset : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        class Package* m_parent = nullptr;
        String new_asset_name;
        bool allow_rename = false;
        int current_index = 0;

        Vector<class Class*> m_filtered_classes;

    public:
        CallBacks<bool(class Class*)> filters;

        ImGuiCreateNewAsset(class Package* pkg, const CallBacks<bool(class Class*)>& = {});
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiRenameObject : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        class Object* m_object = nullptr;
        String new_object_name;
        bool allow_rename = false;

    public:
        ImGuiRenameObject(class Object* object);
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiOpenFile : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        Package* m_package = nullptr;
        Function<void(Package*, const Path&)> m_callback;
        void* m_browser = nullptr;

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


    class ImGuiSpawnNewActor : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        struct Node {
            class Class* self = nullptr;

            struct Compare {
                bool operator()(const Node* a, const Node* b) const;
            };

            TreeSet<Node*> childs;

            ~Node();
        };

        Node* m_root     = nullptr;
        Node* m_selected = nullptr;
        ImGuiID m_dock_id;

        Vector3D m_location = {0, 0, 0};
        Vector3D m_rotation = {0, 0, 0};
        Vector3D m_scale    = {1, 1, 1};
        String m_name;
        bool m_is_open = false;

        void build_tree(Node* node, class Class* self);
        void render_tree(Node* node);
        void render_parameters();
        void begin_dock_space();

    public:
        class World* world = nullptr;

        ImGuiSpawnNewActor(class World* world);

        bool render(RenderViewport* viewport) override;
        static const char* name();

        ~ImGuiSpawnNewActor();
    };

    class ImGuiSceneTree : public ImGuiRenderer::ImGuiAdditionalWindow
    {
        void render_scene_tree(class SceneComponent* component);

    public:
        class World* world             = nullptr;
        class SceneComponent* selected = nullptr;

        CallBacks<void(SceneComponent*)> on_node_select;

        ImGuiSceneTree(SceneComponent* root_component = nullptr);
        bool render(RenderViewport* viewport) override;
        static const char* name();
    };
}// namespace Engine
