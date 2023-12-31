#pragma once
#include <Core/callback.hpp>
#include <additional_window.hpp>

namespace Engine
{
    class ImGuiNotificationMessage : public ImGuiAdditionalWindow
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

    class ImGuiCreateNewPackage : public ImGuiAdditionalWindow
    {
        class Package* _M_parent = nullptr;
        String new_package_name;
        bool allow_rename = false;

    public:
        ImGuiCreateNewPackage(class Package* pkg);
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiRenameObject : public ImGuiAdditionalWindow
    {
        class Object* _M_object = nullptr;
        String new_object_name;
        bool allow_rename = false;

    public:
        ImGuiRenameObject(class Object* object);
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiPackageTree : public ImGuiAdditionalWindow
    {
        bool _M_always_visible;
        class Package* _M_selected = nullptr;
        bool _M_open_package_popup = false;

        void render_internal(Package* package);
        bool render_popup_internal();
        void render_popup();

    public:
        CallBacks<void(Package*)> on_package_select;

        ImGuiPackageTree(bool always_visible = true);
        bool render(class RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiContentBrowser : public ImGuiAdditionalWindow
    {
    private:
        bool _M_show_context_menu = false;


        bool show_context_menu();

    public:
        Package* package = nullptr;

        bool render(RenderViewport* viewport) override;
        static const char* name();
    };

    class ImGuiCreateObject : public ImGuiAdditionalWindow
    {
    private:
        Package* _M_package;
        class Class* _M_current_class = nullptr;
    };
}// namespace Engine
