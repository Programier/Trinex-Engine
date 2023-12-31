#include <Core/constants.hpp>
#include <Core/package.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>
#include <helpers.hpp>


namespace Engine
{
    ImGuiNotificationMessage::ImGuiNotificationMessage(const String& msg, Type type) : _M_message(msg), _M_type(type)
    {}

    bool ImGuiNotificationMessage::render(RenderViewport* viewport)
    {
        bool open                     = true;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImGuiHelpers::construct_vec2<ImVec2>(viewport->size() / 2.0f) - ImVec2(200, 100), ImGuiCond_Once);

        if (ImGui::Begin(name(), &open, window_flags))
        {
            ImVec4 text_color;

            switch (_M_type)
            {
                case Type::Info:
                    text_color = ImVec4(0.0f, 0.7f, 1.0f, 1.0f);
                    break;
                case Type::Warning:
                    text_color = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
                    break;
                case Type::Error:
                    text_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                    break;
            }

            ImGuiRenderer::TextWrappedColored(text_color, "%s", _M_message.c_str());

            static constexpr float button_width  = 80.0f;
            static constexpr float button_height = 25.0f;
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - button_width) * 0.5f);
            ImGui::SetCursorPosY(ImGui::GetWindowSize().y - button_height - 10.0f);

            if (ImGui::Button("OK", ImVec2(button_width, button_height)))
            {
                open = false;
            }

            ImGui::End();
        }

        return open;
    }

    const char* ImGuiNotificationMessage::name()
    {
        return "Notification";
    }

    ImGuiCreateNewPackage::ImGuiCreateNewPackage(Package* parent) : _M_parent(parent ? parent : Object::root_package())
    {}

    bool ImGuiCreateNewPackage::render(class RenderViewport* viewport)
    {
        bool open = true;

        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImGuiHelpers::construct_vec2<ImVec2>(viewport->size() / 2.0f) - ImVec2(200, 100), ImGuiCond_Once);

        ImGui::Begin(name(), &open, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Parent: %s", _M_parent->full_name().c_str());

        ImGuiRenderer::InputText("Package Name", new_package_name);
        ImGui::Checkbox("Allow rename", &allow_rename);

        if (!allow_rename && _M_parent->contains_object(new_package_name))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.0, 0.0, 1.0));
            ImGui::TextWrapped("Cannot create package! Object with name '%s' already exists in package '%s'",
                               new_package_name.c_str(), _M_parent->string_name().c_str());
            ImGui::PopStyleColor();
        }
        else if (new_package_name.find(Constants::name_separator) != String::npos)
        {
            ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot create package! Name can't contain '%s'",
                               Constants::name_separator.c_str());
        }
        else
        {
            ImGui::Separator();

            if (ImGui::Button("Create", ImVec2(100, 25)))
            {
                Package* new_package = Object::new_instance<Package>();
                new_package->name(new_package_name);
                _M_parent->add_object(new_package, allow_rename);
                open = false;
            }
        }

        ImGui::End();
        return open;
    }

    const char* ImGuiCreateNewPackage::name()
    {
        return "Create New Package";
    }

    ImGuiRenameObject::ImGuiRenameObject(Object* object) : _M_object(object)
    {
        if (object)
            new_object_name = object->string_name();
    }

    bool ImGuiRenameObject::render(class RenderViewport* viewport)
    {
        if (!_M_object)
            return false;

        bool open = true;

        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImGuiHelpers::construct_vec2<ImVec2>(viewport->size() / 2.0f) - ImVec2(200, 100), ImGuiCond_Once);

        ImGui::Begin("Rename Object", &open, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Object: %s", _M_object->full_name().c_str());

        ImGuiRenderer::InputText("New Name", new_object_name);
        ImGui::Checkbox("Allow rename", &allow_rename);

        if (_M_object->flag(Object::IsInternal))
        {
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot rename internal object!");
        }
        else if (new_object_name.find(Constants::name_separator) != String::npos)
        {
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot rename object! Name can't contain '%s'",
                                              Constants::name_separator.c_str());
        }
        else
        {
            ImGui::Separator();

            if (ImGui::Button("Rename", ImVec2(100, 25)))
            {
                if (_M_object->name(new_object_name, allow_rename) == ObjectRenameStatus::Failed)
                {
                    list->create<ImGuiNotificationMessage>("Failed to rename object", ImGuiNotificationMessage::Error);
                }

                open = false;
            }
        }

        ImGui::End();
        return open;
    }

    const char* ImGuiRenameObject::name()
    {
        return "Rename Object";
    }


    ImGuiPackageTree::ImGuiPackageTree(bool always_visible) : _M_always_visible(always_visible)
    {}


    bool ImGuiPackageTree::render_popup_internal()
    {
        if (ImGui::Button("Create new package"))
        {
            list->create<ImGuiCreateNewPackage>(_M_selected);
            return false;
        }

        if (!_M_selected->flag(Object::IsInternal) && ImGui::Button("Rename"))
        {
            list->create<ImGuiRenameObject>(_M_selected);
            return false;
        }

        if (!_M_selected->flag(Object::IsInternal) && ImGui::Button("Save"))
        {
            _M_selected->save();
            return false;
        }

        return true;
    }

    void ImGuiPackageTree::render_popup()
    {
        if (!_M_open_package_popup)
            return;

        _M_open_package_popup =
                ImGuiRenderer::BeginPopup("Package Menu##Popup1", 0, &ImGuiPackageTree::render_popup_internal, this);
    }

    void ImGuiPackageTree::render_internal(Package* pkg)
    {
        if (pkg == _M_selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
        }

        bool opened = ImGui::CollapsingHeader(pkg->string_name().c_str());

        if (pkg == _M_selected)
        {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            _M_selected           = pkg;
            _M_open_package_popup = false;
            on_package_select.trigger(_M_selected);
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            _M_selected           = pkg;
            _M_open_package_popup = true;
            on_package_select.trigger(_M_selected);
        }

        if(pkg == _M_selected && _M_open_package_popup)
        {
            ImGui::OpenPopup("Package Menu##Popup1");
        }


        if (opened)
        {
            ImGui::Indent(5.f);
            for (auto& [name, object] : pkg->objects())
            {
                Package* next_package = object->instance_cast<Package>();
                if (next_package)
                {
                    render_internal(next_package);
                }
            }

            ImGui::Unindent(5.0f);
        }
    }

    bool ImGuiPackageTree::render(class RenderViewport* viewport)
    {
        bool open = true;

        if (ImGui::Begin(name(), _M_always_visible ? nullptr : &open))
        {
            render_internal(Object::root_package());
            render_popup();
        }
        ImGui::End();

        return open;
    }

    const char* ImGuiPackageTree::name()
    {
        return "Package Tree";
    }


    bool ImGuiContentBrowser::show_context_menu()
    {
        if (ImGui::Button("Create new package"))
        {
            list->create<ImGuiCreateNewPackage>(package);
            return false;
        }
        return true;
    }

    bool ImGuiContentBrowser::render(RenderViewport* viewport)
    {
        ImGui::Begin(name());
        if (package == nullptr)
        {
            ImGui::Text("No package selected!");
            ImGui::End();
            return true;
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            _M_show_context_menu = true;
        }

        if (_M_show_context_menu)
        {
            ImGui::OpenPopup("##NoName1");
            _M_show_context_menu = ImGuiRenderer::BeginPopup("##NoName1", 0, &ImGuiContentBrowser::show_context_menu, this);
        }


        for (auto& [name, object] : package->objects())
        {
            ImGui::Selectable(object->string_name().c_str());
        }

        ImGui::End();

        return true;
    }

    const char* ImGuiContentBrowser::name()
    {
        return "Content Browser";
    }

}// namespace Engine
