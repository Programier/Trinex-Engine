#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine_config.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/package.hpp>
#include <Graphics/texture_2D.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/imgui_windows.hpp>
#include <icons.hpp>
#include <imgui_internal.h>
#include <theme.hpp>

namespace Engine
{

    using CB = ContentBrowser;

    CB::PackageNodeType CB::PackageTreeNode::type() const
    {
        if (m_is_package)
            return m_package ? PackageNodeType::Package : PackageNodeType::NotLoadedPackage;
        return PackageNodeType::Folder;
    }

    CB::PackageTreeNode* CB::PackageTreeNode::find(const Path& path)
    {
        if (m_path == path)
            return this;

        if (!m_is_builded)
            rebuild();

        for (auto& [name, child] : m_childs)
        {
            auto res = child->find(path);
            if (res)
            {
                return res;
            }
        }

        return nullptr;
    }

    void ContentBrowser::PackageTreeNode::rebuild()
    {
        if (m_package)
        {
            for (auto& [name, object] : m_package->objects())
            {
                Package* pkg = Object::instance_cast<Package>(object);
                if (pkg)
                {
                    PackageTreeNode* new_node = new PackageTreeNode();
                    new_node->m_package      = pkg;
                    new_node->m_name         = pkg->string_name();
                    new_node->m_path         = m_path / new_node->m_name;
                    new_node->m_is_package   = true;

                    m_childs.insert({new_node->m_name, new_node});
                }
            }
        }

        if (m_path.extension() != Constants::package_extention && rootfs()->is_dir(m_path))
        {
            for (auto& entry : VFS::DirectoryIterator(m_path))
            {
                bool is_dir     = rootfs()->is_dir(entry);
                bool is_package = !is_dir && entry.extension() == Constants::package_extention;

                if (!is_dir && !is_package)
                    continue;

                String filename = String(entry.stem());

                if ((m_package && m_package->contains_object(filename)) || m_childs.contains(filename))
                {
                    continue;
                }

                PackageTreeNode*& new_node = m_childs[filename];

                if (new_node == nullptr)
                {
                    new_node = new PackageTreeNode();

                    new_node->m_package = nullptr;
                    new_node->m_path    = m_path / filename;
                    new_node->m_name    = std::move(filename);
                }

                new_node->m_is_package = new_node->m_is_package || is_package;

                m_childs.insert({new_node->m_name, new_node});
            }
        }


        m_is_builded = true;
    }

    void ContentBrowser::PackageTreeNode::clean()
    {
        for (auto& [name, node] : m_childs)
        {
            delete node;
        }
        m_childs.clear();
        m_is_builded = false;
    }

    ContentBrowser::PackageTreeNode::~PackageTreeNode()
    {
        clean();
    }

    void ContentBrowser::init(RenderViewport* viewport)
    {
        m_root                = new PackageTreeNode();
        m_root->m_path       = engine_config.packages_dir;
        m_root->m_package    = Object::root_package();
        m_root->m_name       = m_root->m_package->string_name();
        m_root->m_is_package = true;
        m_selected_package    = m_root;
    }


    bool ContentBrowser::loaded_package_popup(void* data)
    {
        if (!not_loaded_package_popup(data))
            return false;

        if (!folder_package_popup(data))
            return false;

        Package* pkg = selected_package();

        //        if (pkg->is_editable() && ImGui::Button("editor/Rename"_localized))
        //        {
        //            ImGuiRenderer::Window::current()->window_list.create<ImGuiRenameObject>(pkg);
        //            return false;
        //        }

        if (pkg->is_editable() && ImGui::Button("editor/Save"_localized))
        {
            pkg->save();
            return false;
        }

        if (pkg->is_editable() && ImGui::Button("editor/Reload"_localized))
        {
            pkg->load();
            return false;
        }

        return true;
    }

    bool ContentBrowser::not_loaded_package_popup(void* data)
    {
        bool is_editable =
                (!m_show_popup_for->m_package) ||
                (m_show_popup_for->m_package && m_show_popup_for->m_package->is_editable() &&
                 m_show_popup_for->m_package->is_serializable() && !m_show_popup_for->m_package->is_engine_resource());

        if (is_editable && ImGui::Button("editor/Load"_localized))
        {
            String path = Strings::replace_all(m_show_popup_for->m_path.relative(engine_config.packages_dir).str(),
                                               Path::sv_separator, Constants::name_separator);

            Package::load_package(path);
            Path selected_path = std::move(m_selected_package->m_path);
            rebuild_package_tree(selected_path);
            return false;
        }


        return true;
    }

    bool ContentBrowser::folder_package_popup(void* data)
    {
        auto path = Strings::replace_all(m_selected_package->m_path.relative(engine_config.packages_dir).str(),
                                         Path::sv_separator, Constants::name_separator);

        Package* pkg = Object::root_package();

        if (!path.empty())
        {
            pkg = Package::find_package(path, true);
        }

        if (ImGui::Button("editor/Create new package"_localized))
        {
            Function<void(Package*)> callback = [this](Package*) {
                Path selected_path = std::move(m_selected_package->m_path);
                rebuild_package_tree(selected_path);
            };

            ImGuiRenderer::Window::current()->window_list.create<ImGuiCreateNewPackage>(pkg, callback);
            return false;
        }

        return true;
    }

    bool ContentBrowser::render_package_popup(void* data)
    {
        PackageNodeType type = m_show_popup_for->type();

        if (type == PackageNodeType::NotLoadedPackage)
        {
            return not_loaded_package_popup(data);
        }

        if (type == PackageNodeType::Package)
        {
            return loaded_package_popup(data);
        }

        if (type == PackageNodeType::Folder)
        {
            return folder_package_popup(data);
        }

        return false;
    }

    void ContentBrowser::render_package_popup()
    {
        if (m_show_popup_for == nullptr)
            return;

        ImGui::OpenPopup("##popup");
        if (!ImGuiRenderer::BeginPopup("##popup", 0, &ContentBrowser::render_package_popup, this))
            m_show_popup_for = nullptr;
    }

    void ContentBrowser::render_package_tree(PackageTreeNode* node)
    {
        if (!node->m_is_builded)
            node->rebuild();

        if (node == m_selected_package)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
        }

        bool opened = false;

        if (node->m_is_package)
        {
            if (node->m_package == nullptr)
            {
                opened = ImGui::TreeNodeEx(node, ImGuiTreeNodeFlags_CollapsingHeader, "<%s>", node->m_name.c_str());
            }
            else
            {
                opened = ImGui::TreeNodeEx(node, ImGuiTreeNodeFlags_CollapsingHeader, "%s", node->m_name.c_str());
            }
        }
        else
        {
            opened = ImGui::TreeNodeEx(node, ImGuiTreeNodeFlags_CollapsingHeader, "%s", node->m_name.c_str());
        }

        if (node == m_selected_package)
        {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            m_selected_package = node;
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            m_selected_package = node;
            m_show_popup_for   = node;
        }

        if (opened)
        {
            ImGui::Indent(5.f);

            for (auto& [name, child] : node->m_childs)
            {
                render_package_tree(child);
            }

            ImGui::Unindent(5.f);
        }
    }

    void ContentBrowser::render_packages()
    {
        ImGui::Begin("##ContentBrowserPackages"_localized, nullptr, ImGuiWindowFlags_NoTitleBar);
        render_package_tree(m_root);
        render_package_popup();
        ImGui::End();
    }

    bool ContentBrowser::show_context_menu(void* userdata)
    {
        Package* pkg = selected_package();
        if (ImGui::Button("editor/Create new asset"_localized))
        {
            ImGuiRenderer::Window::current()->window_list.create<ImGuiCreateNewAsset>(pkg, filters);
            return false;
        }

        bool is_editable_object = selected_object && selected_object->is_editable();

        if (is_editable_object && ImGui::Button("editor/Rename"_localized))
        {
            ImGuiRenderer::Window::current()->window_list.create<ImGuiRenameObject>(selected_object);
            return false;
        }

        if (is_editable_object && ImGui::Button("editor/Delete"_localized))
        {
            Package* package = selected_object->package();
            package->remove_object(selected_object);
            delete selected_object;
            selected_object = nullptr;
            on_object_select(nullptr);
            return false;
        }
        return true;
    }

    void ContentBrowser::render_content_window()
    {
        const ImVec2 item_size = ImVec2(100, 100) * editor_scale_factor();

        ImGui::Begin("##ContentBrowserItems");
        Package* package = m_selected_package->m_package;

        if (package == nullptr)
        {
            ImGui::Text("%s!", "editor/No package selected"_localized);
            ImGui::End();
            return;
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            m_show_context_menu = true;
        }

        if (m_show_context_menu)
        {
            ImGui::OpenPopup("##NoName1");
            m_show_context_menu = ImGuiRenderer::BeginPopup("##NoName1", 0, &ContentBrowser::show_context_menu, this);
        }

        ImVec2 content_size = ImGui::GetContentRegionAvail();
        bool not_first_item = false;

        float padding = ImGui::GetStyle().FramePadding.x;

        size_t rendered = 0;

        for (auto& [name, object] : package->objects())
        {
            bool in_filter = filters.empty();

            if (!in_filter)
            {
                for (auto& [name, callback] : filters.callbacks())
                {
                    in_filter = callback(object->class_instance());
                    if (in_filter)
                        break;
                }
            }

            if (!in_filter)
                continue;

            ++rendered;

            if (not_first_item)
            {
                ImGui::SameLine();

                if (ImGui::GetCursorPosX() + item_size.x >= content_size.x)
                {
                    ImGui::NewLine();
                    ImGui::NewLine();
                }
            }
            else
            {
                not_first_item = true;
            }

            ImGui::BeginGroup();

            ImGuiRenderer::ImGuiTexture* imgui_texture = Icons::find_imgui_icon(object);

            if (imgui_texture && imgui_texture->handle())
            {
                ImGui::PushID(name.to_string().c_str());

                bool is_selected = selected_object == object;

                if (is_selected)
                {
                    static ImVec4 color1 = ImVec4(79.f / 255.f, 109.f / 255.f, 231.f / 255.f, 1.0),
                                  color2 = ImVec4(114.f / 255.f, 138.f / 255.f, 233.f / 255.f, 1.0);

                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(color1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(color2));
                }

                ImVec2 item_start = ImGui::GetCursorPos();

                bool is_single_press = ImGui::ImageButton(imgui_texture->handle(), item_size);
                bool is_double_press = false;

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    is_single_press = false;
                    is_double_press = true;
                }

                if (is_single_press)
                {
                    selected_object = object;
                    on_object_select(object);
                }
                else if (is_double_press)
                {
                    on_object_double_click(object);
                }


                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("ContendBrowser->Object", &object, sizeof(Object**));
                    ImVec2 item_start = ImGui::GetCursorPos();
                    ImGui::Image(imgui_texture->handle(), item_size);

                    if (imgui_texture->texture() == Icons::default_texture())
                    {
                        const char* class_name = object->class_instance()->base_name_splitted().c_str();
                        ImVec2 text_size       = ImGui::CalcTextSize(class_name, nullptr, false, item_size.x);

                        ImVec2 text_pos = item_start + ImVec2(((item_size.x / 2) - (text_size.x / 2)) + padding,
                                                              (item_size.y / 2) - (text_size.y / 2));

                        ImGui::SetCursorPos(text_pos);
                        ImGui::PushTextWrapPos(text_pos.x + item_size.x);

                        ImGui::TextWrapped("%s", class_name);
                        ImGui::PopTextWrapPos();
                    }

                    ImGui::EndDragDropSource();
                }

                if (is_selected)
                {
                    ImGui::PopStyleColor(2);
                }

                ImVec2 current_pos = ImGui::GetCursorPos();

                if (imgui_texture->texture() == Icons::default_texture())
                {
                    const char* class_name = object->class_instance()->base_name_splitted().c_str();
                    ImVec2 text_size       = ImGui::CalcTextSize(class_name, nullptr, false, item_size.x);

                    ImVec2 text_pos = item_start + ImVec2(((item_size.x / 2) - (text_size.x / 2)) + padding,
                                                          (item_size.y / 2) - (text_size.y / 2));

                    ImGui::SetCursorPos(text_pos);
                    ImGui::PushTextWrapPos(text_pos.x + item_size.x);

                    ImGui::TextWrapped("%s", class_name);
                    ImGui::PopTextWrapPos();
                }


                String object_name = Strings::make_sentence(name);
                float offset = (item_size.x - ImGui::CalcTextSize(object_name.c_str(), nullptr, false, item_size.x).x) / 2.f;
                current_pos.x += offset;
                ImGui::SetCursorPos(current_pos);

                ImGui::PushTextWrapPos(current_pos.x + item_size.x - offset);
                ImGui::TextWrapped("%s", object_name.c_str());
                ImGui::PopTextWrapPos();

                ImGui::PopID();
            }
            else
            {
                if (ImGui::Selectable(name.c_str(), selected_object == object, 0, item_size))
                {
                    selected_object = object;
                    on_object_select(object);
                }
            }

            ImGui::EndGroup();
        }

        if (rendered == 0)
        {
            ImGui::Text("%s", "editor/No objects found"_localized);
        }


        ImGui::End();
    }

    void ContentBrowser::create_dock_space()
    {
        m_dock_window_id = ImGui::GetID("##ContentBrowserDockSpace");

        ImGui::DockSpace(m_dock_window_id, {0, 0},
                         ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoUndocking | ImGuiDockNodeFlags_NoTabBar);

        if (frame_number == 1)
        {
            ImGui::DockBuilderRemoveNode(m_dock_window_id);
            ImGui::DockBuilderAddNode(m_dock_window_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(m_dock_window_id, ImGui::GetWindowSize());

            auto dock_id_left = ImGui::DockBuilderSplitNode(m_dock_window_id, ImGuiDir_Left, 0.2f, nullptr, &m_dock_window_id);

            ImGui::DockBuilderDockWindow("##ContentBrowserPackages", dock_id_left);
            ImGui::DockBuilderDockWindow("##ContentBrowserItems", m_dock_window_id);
            ImGui::DockBuilderFinish(m_dock_window_id);
        }
    }


    void ContentBrowser::rebuild_package_tree(const Path& selected)
    {
        m_root->clean();
        m_show_popup_for   = nullptr;
        m_selected_package = m_root->find(selected);

        if (m_selected_package == nullptr)
        {
            m_selected_package = m_root;
        }
    }

    bool ContentBrowser::render(RenderViewport* viewport)
    {
        bool open = true;
        ImGui::Begin(name(), closable ? &open : nullptr);
        create_dock_space();

        render_packages();
        render_content_window();

        ImGui::End();
        return open;
    }

    Package* ContentBrowser::selected_package() const
    {
        return m_selected_package ? m_selected_package->m_package : nullptr;
    }

    const char* ContentBrowser::name()
    {
        return "editor/Content Browser Title"_localized;
    }

    ContentBrowser::~ContentBrowser()
    {
        delete m_root;
    }
}// namespace Engine
