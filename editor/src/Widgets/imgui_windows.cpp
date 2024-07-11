#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/package.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>
#include <Platform/platform.hpp>
#include <Widgets/imgui_windows.hpp>
#include <icons.hpp>
#include <imfilebrowser.h>
#include <imgui_internal.h>
#include <theme.hpp>

namespace Engine
{
#define text_wrapped_colored(color, format, ...)                                                                                 \
    ImGui::PushStyleColor(ImGuiCol_Text, color);                                                                                 \
    ImGui::TextWrapped(format __VA_OPT__(, ) __VA_ARGS__);                                                                       \
    ImGui::PopStyleColor()


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


            text_wrapped_colored(text_color, "%s", m_message.c_str());

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
            text_wrapped_colored(ImVec4(1.0, 0.0, 0.0, 1.0),
                                 "Cannot create package! Object with name '%s' already exists in package '%s'",
                                 new_package_name.c_str(), m_parent->string_name().c_str());
        }
        else if (new_package_name.find(Constants::name_separator) != String::npos)
        {
            text_wrapped_colored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot create package! Name can't contain '%s'",
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

                if (m_on_create)
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
            text_wrapped_colored(ImVec4(1.0, 0.0, 0.0, 1.0),
                                 "Cannot create new Asset! Object with name '%s' already exists in package '%s'",
                                 new_asset_name.c_str(), m_parent->string_name().c_str());
        }
        else if (new_asset_name.find(Constants::name_separator) != String::npos)
        {
            text_wrapped_colored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot create asset! Name can't contain '%s'",
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
            text_wrapped_colored(ImVec4(1.0, 0.0, 0.0, 1.0), "Cannot rename internal object!");
        }
        else if (new_object_name.empty())
        {
            text_wrapped_colored(ImVec4(1.0, 0.0, 0.0, 1.0), "Name can't be empty!");
        }
        else if (new_object_name.starts_with(Constants::name_separator))
        {
            text_wrapped_colored(ImVec4(1.0, 0.0, 0.0, 1.0), "Name can't starts with '%s'!", Constants::name_separator.c_str());
        }
        else if (new_object_name.ends_with(Constants::name_separator))
        {
            text_wrapped_colored(ImVec4(1.0, 0.0, 0.0, 1.0), "Name can't ends with '%s'!", Constants::name_separator.c_str());
        }
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


    ImGuiOpenFile::ImGuiOpenFile(Flags<Flag> flags) : m_flags(flags)
    {
        m_browser = new ImGui::FileBrowser(static_cast<ImGuiFileBrowserFlags>(flags));
        m_browser->SetTitle(name());
        m_browser->Open();
    }

    ImGuiOpenFile& ImGuiOpenFile::window_pos(int_t posx, int_t posy) noexcept
    {
        m_browser->SetWindowPos(posx, posy);
        return *this;
    }

    ImGuiOpenFile& ImGuiOpenFile::window_size(int_t width, int_t height) noexcept
    {
        m_browser->SetWindowSize(width, height);
        return *this;
    }

    ImGuiOpenFile& ImGuiOpenFile::title(StringView title)
    {
        m_browser->SetTitle(String(title));
        return *this;
    }

    bool ImGuiOpenFile::has_selected() const noexcept
    {
        return m_browser->HasSelected();
    }

    ImGuiOpenFile& ImGuiOpenFile::clear_selected()
    {
        m_browser->ClearSelected();
        return *this;
    }

    ImGuiOpenFile& ImGuiOpenFile::current_type_filter_index(int_t index)
    {
        m_browser->SetCurrentTypeFilterIndex(index);
        return *this;
    }

    ImGuiOpenFile& ImGuiOpenFile::input_name(StringView input)
    {
        m_browser->SetInputName(input);
        return *this;
    }

    ImGuiOpenFile& ImGuiOpenFile::type_filters(const Vector<String>& type_filters)
    {
        m_browser->SetTypeFilters(type_filters);
        return *this;
    }

    ImGuiOpenFile& ImGuiOpenFile::pwd(const Path& path)
    {
        m_browser->SetPwd(rootfs()->native_path(path).str());
        return *this;
    }

    bool ImGuiOpenFile::render(RenderViewport* viewport)
    {
        m_browser->Display();

        if (m_browser->HasSelected())
        {
            if ((m_flags & Flags(Flag::MultipleSelection)) == Flag::MultipleSelection)
            {
                for (auto& path : m_browser->GetMultiSelected())
                {
                    on_select(Path(std::filesystem::relative(path).string()));
                }
            }
            else
            {
                on_select(Path(std::filesystem::relative(m_browser->GetSelected()).string()));
            }

            return false;
        }

        if (!m_browser->IsOpened())
            return false;
        return true;
    }

    ImGuiOpenFile::~ImGuiOpenFile()
    {
        delete m_browser;
    }


    const char* ImGuiOpenFile::name()
    {
        return "editor/Open File"_localized;
    }


    bool ImGuiSpawnNewActor::Node::Compare::operator()(const Node* a, const Node* b) const
    {
        return a->self->base_name_splitted() < b->self->base_name_splitted();
    }

    ImGuiSpawnNewActor::Node::~Node()
    {
        for (auto& child : childs)
        {
            delete child;
        }
    }

    void ImGuiSpawnNewActor::build_tree(Node* node, class Class* self)
    {
        node->self = self;

        for (Struct* child : self->childs())
        {
            if (child->is_class())
            {
                Node* child_node = new Node();
                build_tree(child_node, reinterpret_cast<Class*>(child));
                node->childs.insert(child_node);
            }
        }
    }

    ImGuiSpawnNewActor::ImGuiSpawnNewActor(class World* world) : world(world)
    {
        Class* actor_class = Class::static_find("Engine::Actor", true);
        m_root             = new Node();
        build_tree(m_root, actor_class);
        m_monitor_size = Platform::monitor_info().size;
    }

    ImGuiSpawnNewActor::~ImGuiSpawnNewActor()
    {
        delete m_root;
    }

    void ImGuiSpawnNewActor::render_tree(Node* node)
    {
        bool state = ImGui::TreeNodeEx(node->self->base_name_splitted().c_str(),
                                       (node == m_selected ? ImGuiTreeNodeFlags_Selected : 0));

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            m_selected = node;
        }

        if (state)
        {
            for (Node* child : node->childs)
            {
                render_tree(child);
            }

            ImGui::TreePop();
        }
    }

    void ImGuiSpawnNewActor::begin_dock_space()
    {
        m_dock_id = ImGui::GetID("##SpawnActorDock");

        ImGui::DockSpace(m_dock_id, {0, 0},
                         ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoUndocking | ImGuiDockNodeFlags_NoTabBar);

        if (frame_number == 1)
        {
            ImGui::DockBuilderRemoveNode(m_dock_id);
            ImGui::DockBuilderAddNode(m_dock_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(m_dock_id, ImGui::GetWindowSize());

            auto dock_id_left = ImGui::DockBuilderSplitNode(m_dock_id, ImGuiDir_Left, 0.35f, nullptr, &m_dock_id);

            ImGui::DockBuilderDockWindow("##ActorSpawner->ActorParameters", dock_id_left);
            ImGui::DockBuilderDockWindow("##ActorSpawner->ActorSelector", m_dock_id);
            ImGui::DockBuilderFinish(m_dock_id);
        }
    }

    void ImGuiSpawnNewActor::render_parameters()
    {
        ImGui::Text("editor/Class: %s"_localized, m_selected ? m_selected->self->base_name_splitted().c_str() : "None");
        ImGuiRenderer::InputText("editor/Name"_localized, m_name);
        ImGui::InputFloat3("editor/Location"_localized, &m_location.x);
        ImGui::InputFloat3("editor/Rotation"_localized, &m_rotation.x);
        ImGui::InputFloat3("editor/Scale"_localized, &m_scale.x);

        if (m_selected && ImGui::Button("editor/Spawn"_localized))
        {
            world->spawn_actor(m_selected->self, m_location, m_rotation, m_scale, m_name);
            m_is_open = false;
        }
    }

    bool ImGuiSpawnNewActor::render(RenderViewport* viewport)
    {
        m_is_open = true;
        ImGui::SetNextWindowPos(ImGuiHelpers::construct_vec2<ImVec2>(m_monitor_size / 2.f), ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::SetNextWindowSize({900, 450}, ImGuiCond_Appearing);
        ImGui::Begin(name(), &m_is_open);

        begin_dock_space();

        ImGui::Begin("##ActorSpawner->ActorSelector");
        render_tree(m_root);
        ImGui::End();


        ImGui::Begin("##ActorSpawner->ActorParameters");
        render_parameters();
        ImGui::End();


        ImGui::End();
        return m_is_open;
    }

    const char* ImGuiSpawnNewActor::name()
    {
        return "editor/Spawn Actor"_localized;
    }
}// namespace Engine
