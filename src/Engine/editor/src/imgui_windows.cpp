#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/package.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>
#include <imfilebrowser.h>
#include <imgui_class_property.hpp>
#include <imgui_windows.hpp>

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
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0),
                                              "Cannot create package! Object with name '%s' already exists in package '%s'",
                                              new_package_name.c_str(), _M_parent->string_name().c_str());
        }
        else if (new_package_name.find(Constants::name_separator) != String::npos)
        {
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot create package! Name can't contain '%s'",
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
        return "New Package";
    }

    ImGuiCreateNewAsset::ImGuiCreateNewAsset(class Package* pkg) : _M_parent(pkg)
    {}

    static const char* get_asset_class_name_by_index(void* userdata, int index)
    {
        return Class::asset_classes()[index]->name().c_str();
    }

    bool ImGuiCreateNewAsset::render(class RenderViewport* viewport)
    {
        bool open = true;

        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImGuiHelpers::construct_vec2<ImVec2>(viewport->size() / 2.0f) - ImVec2(200, 100), ImGuiCond_Once);

        ImGui::Begin(name(), &open, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Parent: %s", _M_parent->full_name().c_str());

        ImGui::Combo("Class", &current_index, get_asset_class_name_by_index, nullptr, Class::asset_classes().size());

        ImGuiRenderer::InputText("Asset Name", new_asset_name);
        ImGui::Checkbox("Allow rename", &allow_rename);

        if (!allow_rename && _M_parent->contains_object(new_asset_name))
        {
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0),
                                              "Cannot create new Asset! Object with name '%s' already exists in package '%s'",
                                              new_asset_name.c_str(), _M_parent->string_name().c_str());
        }
        else if (new_asset_name.find(Constants::name_separator) != String::npos)
        {
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot create asset! Name can't contain '%s'",
                                              Constants::name_separator.c_str());
        }
        else
        {
            ImGui::Separator();

            if (ImGui::Button("Create", ImVec2(100, 25)))
            {
                Class* class_instance  = Class::asset_classes()[current_index];
                Object* created_object = class_instance->create_object();
                created_object->name(new_asset_name);
                _M_parent->add_object(created_object);
                open = false;
            }
        }

        ImGui::End();
        return open;
    }

    const char* ImGuiCreateNewAsset::name()
    {
        return "New Asset";
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

        if (_M_object->flags(Object::IsInternal))
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
                    ImGuiRenderer::Window::current()->window_list.create<ImGuiNotificationMessage>(
                            "Failed to rename object", ImGuiNotificationMessage::Error);
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


    bool ImGuiPackageTree::render_popup_internal(void* userdata)
    {
        if (ImGui::Button("Create new package"))
        {
            ImGuiRenderer::Window::current()->window_list.create<ImGuiCreateNewPackage>(_M_selected);
            return false;
        }

        if (!_M_selected->flags(Object::IsInternal) && ImGui::Button("Rename"))
        {
            ImGuiRenderer::Window::current()->window_list.create<ImGuiRenameObject>(_M_selected);
            return false;
        }

        if (!_M_selected->flags(Object::IsInternal) && ImGui::Button("Save"))
        {
            _M_selected->save();
            return false;
        }

        return true;
    }

    void ImGuiPackageTree::render_popup(RenderViewport* viewport)
    {
        if (!_M_open_package_popup)
            return;

        _M_open_package_popup =
                ImGuiRenderer::BeginPopup("Package Menu##Popup1", 0, &ImGuiPackageTree::render_popup_internal, this, viewport);
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

        if (pkg == _M_selected && _M_open_package_popup)
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
            render_popup(viewport);
        }
        ImGui::End();

        return open;
    }

    Package* ImGuiPackageTree::selected_package() const
    {
        return _M_selected;
    }

    const char* ImGuiPackageTree::name()
    {
        return "Package Tree";
    }


    ImGuiOpenFile::ImGuiOpenFile(Package* pkg, const Function<void(Package*, const Path&)>& callback,
                                 const Vector<String>& type_filters)
        : _M_package(pkg), _M_callback(callback)
    {
        auto* browser = new ImGui::FileBrowser();
        browser->SetTitle(name());
        if (!type_filters.empty())
            browser->SetTypeFilters(type_filters);

        browser->Open();
        _M_browser = browser;
    }

    bool ImGuiOpenFile::render(RenderViewport* viewport)
    {
        ImGui::FileBrowser* browser = reinterpret_cast<ImGui::FileBrowser*>(_M_browser);

        browser->Display();

        if (browser->HasSelected())
        {
            _M_callback(_M_package, browser->GetSelected());
            return false;
        }

        if (!browser->IsOpened())
            return false;
        return true;
    }

    ImGuiOpenFile::~ImGuiOpenFile()
    {
        delete reinterpret_cast<ImGui::FileBrowser*>(_M_browser);
    }

    const char* ImGuiOpenFile::name()
    {
        return "Open File";
    }

    bool ImGuiObjectProperties::render(RenderViewport* viewport)
    {
        ImGui::Begin(name());
        if (object)
        {
            ImGui::Text("Object: %s", object->name().to_string().c_str());
            ImGui::Text("Class: %s", object->class_instance()->name().c_str());
            ImGui::Separator();

            for (Class* self = object->class_instance(); self; self = self->parent())
            {
                if (!self->properties().empty())
                {
                    if (ImGui::CollapsingHeader(self->name().c_str()))
                    {
                        ImGui::Indent(5.f);
                        for (Property* prop : self->properties())
                        {
                            render_object_property(object, prop, true);
                        }
                        ImGui::Unindent(5.f);
                    }
                }
            }
        }
        ImGui::End();

        return true;
    }

    const char* ImGuiObjectProperties::name()
    {
        return "Properties";
    }


    ImGuiSceneTree::ImGuiSceneTree(SceneComponent* root_component) : root_component(root_component)
    {}


    void ImGuiSceneTree::render_scene_tree(class SceneComponent* component)
    {}

    bool ImGuiSceneTree::render(RenderViewport* viewport)
    {
        ImGui::Begin(name());
        render_scene_tree(root_component);
        ImGui::End();

        return true;
    }

    const char* ImGuiSceneTree::name()
    {
        return "Scene Tree";
    }

}// namespace Engine