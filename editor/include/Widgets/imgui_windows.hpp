#pragma once
#include <Core/callback.hpp>
#include <Core/userdata.hpp>
#include <Graphics/imgui.hpp>

namespace ImGui
{
    class FileBrowser;
}

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
        ImGui::FileBrowser* m_browser = nullptr;


    public:
        enum Flag
        {
            None              = 0,
            SelectDirectory   = 1 << 0,
            EnterNewFilename  = 1 << 1,
            NoModal           = 1 << 2,
            NoTitleBar        = 1 << 3,
            NoStatusBar       = 1 << 4,
            CloseOnEsc        = 1 << 5,
            CreateNewDir      = 1 << 6,
            MultipleSelection = 1 << 7,
        };

    private:
        Flags<Flag> m_flags;

    public:
        CallBacks<void(const Path&)> on_select;

        ImGuiOpenFile(Flags<Flag> flags = 0);
        ImGuiOpenFile& window_pos(int_t posx, int_t posy) noexcept;
        ImGuiOpenFile& window_size(int_t width, int_t height) noexcept;
        ImGuiOpenFile& title(StringView title);
        bool has_selected() const noexcept;
        ImGuiOpenFile& clear_selected();
        ImGuiOpenFile& current_type_filter_index(int_t index);
        ImGuiOpenFile& input_name(StringView input);
        ImGuiOpenFile& type_filters(const Vector<String>& type_filters);
        ImGuiOpenFile& pwd(const Path& path);
        bool render(RenderViewport* viewport) override;
        static const char* name();

        ~ImGuiOpenFile();
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

        Size2D m_monitor_size;
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
}// namespace Engine
