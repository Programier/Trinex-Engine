#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/package.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/visual_material.hpp>
#include <PropertyRenderers/imgui_class_property.hpp>
#include <Widgets/imgui_windows.hpp>
#include <imfilebrowser.h>

namespace Engine
{
    ImGuiNotificationMessage::ImGuiNotificationMessage(const String& msg, Type type) : m_message(msg), m_type(type)
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

            switch (m_type)
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

            ImGuiRenderer::TextWrappedColored(text_color, "%s", m_message.c_str());

            static constexpr float button_width  = 80.0f;
            static constexpr float button_height = 25.0f;
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - button_width) * 0.5f);
            ImGui::SetCursorPosY(ImGui::GetWindowSize().y - button_height - 10.0f);

            if (ImGui::Button("editor/Ok"_localized, ImVec2(button_width, button_height)))
            {
                open = false;
            }

            ImGui::End();
        }

        return open;
    }

    const char* ImGuiNotificationMessage::name()
    {
        return "editor/Notification Title"_localized;
    }

    ImGuiCreateNewPackage::ImGuiCreateNewPackage(Package* parent, const CallBack<void(Package*)>& on_create)
        : m_parent(parent ? parent : Object::root_package()), m_on_create(on_create)
    {}

    bool ImGuiCreateNewPackage::render(class RenderViewport* viewport)
    {
        bool open = true;

        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImGuiHelpers::construct_vec2<ImVec2>(viewport->size() / 2.0f) - ImVec2(200, 100), ImGuiCond_Once);

        ImGui::Begin(name(), closable ? &open : nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Parent: %s", m_parent->full_name().c_str());

        ImGuiRenderer::InputText("editor/Package Name"_localized, new_package_name);
        ImGui::Checkbox("editor/Allow rename"_localized, &allow_rename);

        if (!allow_rename && m_parent->contains_object(new_package_name))
        {
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0),
                                              "Cannot create package! Object with name '%s' already exists in package '%s'",
                                              new_package_name.c_str(), m_parent->string_name().c_str());
        }
        else if (new_package_name.find(Constants::name_separator) != String::npos)
        {
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot create package! Name can't contain '%s'",
                                              Constants::name_separator.c_str());
        }
        else
        {
            ImGui::Separator();

            if (ImGui::Button("editor/Create"_localized, ImVec2(100, 25)))
            {
                Package* new_package = Object::new_instance<Package>();
                new_package->name(new_package_name);
                m_parent->add_object(new_package, allow_rename);
                open = false;

                m_on_create(new_package);
            }
        }

        ImGui::End();
        return open;
    }

    const char* ImGuiCreateNewPackage::name()
    {
        return "editor/New Package Title"_localized;
    }

    ImGuiCreateNewAsset::ImGuiCreateNewAsset(class Package* pkg, const CallBacks<bool(class Class*)>& filters)
        : m_parent(pkg), filters(filters)
    {
        if (filters.empty())
        {
            m_filtered_classes = Class::asset_classes();
        }
        else
        {
            for (Class* class_instance : Class::asset_classes())
            {
                for (auto& [id, filter] : filters.callbacks())
                {
                    if (filter(class_instance))
                    {
                        m_filtered_classes.push_back(class_instance);
                        break;
                    }
                }
            }
        }
    }

    static const char* get_asset_class_name_default(void* userdata, int index)
    {
        return (*reinterpret_cast<Vector<Class*>*>(userdata))[index]->name().c_str();
    }

    bool ImGuiCreateNewAsset::render(class RenderViewport* viewport)
    {
        bool open = true;

        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImGuiHelpers::construct_vec2<ImVec2>(viewport->size() / 2.0f) - ImVec2(200, 100), ImGuiCond_Once);

        ImGui::Begin(name(), closable ? &open : nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Parent: %s", m_parent->full_name().c_str());

        ImGui::Combo("editor/Class"_localized, &current_index, get_asset_class_name_default, &m_filtered_classes,
                     m_filtered_classes.size());

        ImGuiRenderer::InputText("editor/Asset Name"_localized, new_asset_name);
        ImGui::Checkbox("editor/Allow rename"_localized, &allow_rename);

        if (!allow_rename && m_parent->contains_object(new_asset_name))
        {
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0),
                                              "Cannot create new Asset! Object with name '%s' already exists in package '%s'",
                                              new_asset_name.c_str(), m_parent->string_name().c_str());
        }
        else if (new_asset_name.find(Constants::name_separator) != String::npos)
        {
            ImGuiRenderer::TextWrappedColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot create asset! Name can't contain '%s'",
                                              Constants::name_separator.c_str());
        }
        else
        {
            ImGui::Separator();

            if (ImGui::Button("editor/Create"_localized, ImVec2(100, 25)))
            {
                Class* class_instance  = m_filtered_classes[current_index];
                Object* created_object = class_instance->create_object();
                created_object->name(new_asset_name);
                m_parent->add_object(created_object);
                open = false;
            }
        }

        ImGui::End();
        return open;
    }

    const char* ImGuiCreateNewAsset::name()
    {
        return "editor/New Asset Title"_localized;
    }

    ImGuiRenameObject::ImGuiRenameObject(Object* object) : m_object(object)
    {
        if (object)
            new_object_name = object->string_name();
    }

    bool ImGuiRenameObject::render(class RenderViewport* viewport)
    {
        if (!m_object)
            return false;

        bool open = true;

        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImGuiHelpers::construct_vec2<ImVec2>(viewport->size() / 2.0f) - ImVec2(200, 100), ImGuiCond_Once);

        ImGui::Begin(name(), closable ? &open : nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Object: %s", m_object->full_name().c_str());

        ImGuiRenderer::InputText("editor/New Name"_localized, new_object_name);
        ImGui::Checkbox("editor/Allow rename"_localized, &allow_rename);

        if (!m_object->is_editable())
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

            if (ImGui::Button("editor/Rename"_localized))
            {
                if (m_object->name(new_object_name, allow_rename) == ObjectRenameStatus::Failed)
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
        return "editor/Rename Object Title"_localized;
    }


    ImGuiOpenFile::ImGuiOpenFile(Package* pkg, const Function<void(Package*, const Path&)>& callback,
                                 const Vector<String>& type_filters)
        : m_package(pkg), m_callback(callback)
    {
        auto* browser = new ImGui::FileBrowser();
        browser->SetTitle(name());
        if (!type_filters.empty())
            browser->SetTypeFilters(type_filters);

        browser->Open();
        m_browser = browser;
    }

    bool ImGuiOpenFile::render(RenderViewport* viewport)
    {
        ImGui::FileBrowser* browser = reinterpret_cast<ImGui::FileBrowser*>(m_browser);

        browser->Display();

        if (browser->HasSelected())
        {
            m_callback(m_package, Path(browser->GetSelected().string()));
            return false;
        }

        if (!browser->IsOpened())
            return false;
        return true;
    }

    ImGuiOpenFile::~ImGuiOpenFile()
    {
        delete reinterpret_cast<ImGui::FileBrowser*>(m_browser);
    }


    const char* ImGuiOpenFile::name()
    {
        return "editor/Open File Title"_localized;
    }

    bool ImGuiObjectProperties::render(RenderViewport* viewport)
    {
        bool open = true;
        ImGui::Begin(name(), closable ? &open : nullptr);
        if (object)
        {
            ImGui::Text("editor/Object: %s"_localized, object->name().to_string().c_str());
            ImGui::Text("editor/Class: %s"_localized, object->class_instance()->name().c_str());
            if (ImGui::Button("editor/Apply changes"_localized))
            {
                object->apply_changes();
            }
            ImGui::Separator();

            render_object_properties(object);
        }
        ImGui::End();

        return open;
    }

    const char* ImGuiObjectProperties::name()
    {
        return "editor/Properties Title"_localized;
    }


    ImGuiSceneTree::ImGuiSceneTree(SceneComponent* root_component) : world(nullptr)
    {}


    void ImGuiSceneTree::render_scene_tree(class SceneComponent* component)
    {
        if (!component)
            return;

        if (component == selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
        }

        bool opened = ImGui::TreeNodeEx(component, ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_OpenOnArrow, "%s",
                                        component->name().c_str());

        if (component == selected)
        {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            selected = component;
            on_node_select(component);
        }

        if (opened)
        {
            ImGui::Indent(5.f);

            for (auto& child : component->childs())
            {
                render_scene_tree(child);
            }

            ImGui::Unindent(5.f);
        }
    }

    bool ImGuiSceneTree::render(RenderViewport* viewport)
    {
        bool open = true;
        ImGui::Begin(name(), closable ? &open : nullptr);
        render_scene_tree(world->scene()->root_component());
        ImGui::End();

        return open;
    }

    const char* ImGuiSceneTree::name()
    {
        return "editor/Scene Tree Title"_localized;
    }
}// namespace Engine
