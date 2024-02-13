#include <Clients/material_editor_client.hpp>
#include <Clients/open_client.hpp>
#include <Compiler/compiler.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/group.hpp>
#include <Core/localization.hpp>
#include <Core/package.hpp>
#include <Core/thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/visual_material.hpp>
#include <Window/window.hpp>
#include <dock_window.hpp>
#include <editor_config.hpp>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <imgui_windows.hpp>
#include <theme.hpp>

#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>


#define MATERIAL_EDITOR_DEBUG 1

namespace Engine
{
    implement_engine_class_default_init(MaterialEditorClient);


    static bool is_material(Class* class_instance)
    {
        if (class_instance->is_a(MaterialInterface::static_class_instance()))
            return true;
        return false;
    }

    void MaterialEditorClient::on_package_tree_close()
    {
        _M_package_tree = nullptr;
    }

    void MaterialEditorClient::on_content_browser_close()
    {
        _M_content_browser = nullptr;
    }

    void MaterialEditorClient::on_properties_window_close()
    {
        _M_properties = nullptr;
    }


    MaterialEditorClient& MaterialEditorClient::create_package_tree()
    {
        _M_package_tree = ImGuiRenderer::Window::current()->window_list.create<ImGuiPackageTree>();
        _M_package_tree->on_package_select.push(std::bind(&MaterialEditorClient::on_package_select, this, std::placeholders::_1));
        _M_package_tree->on_close.push(std::bind(&MaterialEditorClient::on_package_tree_close, this));
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::create_content_browser()
    {
        _M_content_browser = ImGuiRenderer::Window::current()->window_list.create<ImGuiContentBrowser>();
        _M_content_browser->on_close.push(std::bind(&MaterialEditorClient::on_content_browser_close, this));
        _M_content_browser->on_object_selected.push(
                std::bind(&MaterialEditorClient::on_object_select, this, std::placeholders::_1));

        _M_content_browser->filters.push(is_material);

        if (_M_package_tree)
        {
            _M_content_browser->package = _M_package_tree->selected_package();
        }
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::create_properties_window()
    {
        _M_properties = ImGuiRenderer::Window::current()->window_list.create<ImGuiObjectProperties>();
        _M_properties->on_close.push(std::bind(&MaterialEditorClient::on_properties_window_close, this));

        if (_M_content_browser)
        {
            on_object_select(_M_content_browser->selected);
        }
        return *this;
    }


    MaterialEditorClient& MaterialEditorClient::on_bind_to_viewport(class RenderViewport* viewport)
    {
        Window* window = viewport->window();
        if (window == nullptr)
        {
            throw EngineException("Cannot bind client to non-window viewport!");
        }

        window->imgui_initialize(initialize_theme);
        String new_title = Strings::format("Trinex Material Editor [{} RHI]", engine_instance->rhi()->name().c_str());
        window->title(new_title);

        engine_instance->thread(ThreadType::RenderThread)->wait_all();
        _M_viewport = viewport;


        ImGuiRenderer::Window* imgui_window = window->imgui_window();
        ImGuiRenderer::Window* prev_window  = ImGuiRenderer::Window::current();
        ImGuiRenderer::Window::make_current(imgui_window);

        create_package_tree().create_properties_window().create_content_browser();

        // Create imgui node editor context
        ax::NodeEditor::Config config;
        config.NavigateButtonIndex = ImGuiMouseButton_Middle;
        config.SettingsFile        = nullptr;

        auto context = ax::NodeEditor::CreateEditor(&config);
        imgui_window->on_destroy.push([context]() { ax::NodeEditor::DestroyEditor(context); });


        _M_editor_context = context;

        ImGuiRenderer::Window::make_current(prev_window);

        Class* instance = Class::static_find(editor_config.material_compiler);
        if (instance)
        {
            auto obj    = instance->create_object();
            _M_compiler = obj->instance_cast<ShaderCompiler>();
        }

        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render(class RenderViewport* viewport)
    {
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }

    void MaterialEditorClient::on_package_select(Package* package)
    {
        if (_M_content_browser)
        {
            _M_content_browser->package = package;
        }
    }

    void MaterialEditorClient::on_object_select(Object* object)
    {
        _M_current_material = Object::instance_cast<VisualMaterial>(object);
        if (_M_properties)
        {
            _M_properties->object = object;
        }
    }

    void MaterialEditorClient::render_dock_window(void* userdata)
    {
        auto dock_id                       = ImGui::GetID("MaterialEditorDock##Dock");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("editor/View"_localized))
            {
                if (ImGui::MenuItem("editor/Open Editor"_localized))
                {
                    open_editor();
                }

                if (ImGui::MenuItem("editor/Open Package Tree"_localized, nullptr, false, _M_package_tree == nullptr))
                {
                    create_package_tree();
                }

                if (ImGui::MenuItem("editor/Open Content Browser"_localized, nullptr, false, _M_content_browser == nullptr))
                {
                    create_content_browser();
                }

                if (ImGui::MenuItem("editor/Open Properties Window"_localized, nullptr, false, _M_properties == nullptr))
                {
                    create_properties_window();
                }

                ImGui::Checkbox("editor/Open Material Code"_localized, &_M_open_material_code_window);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("editor/Edit"_localized))
            {
                if (ImGui::MenuItem("editor/Reload localization"_localized))
                {
                    Localization::instance()->reload();
                }

                if (ImGui::BeginMenu("editor/Change language"_localized))
                {
                    for (const String& lang : engine_config.languages)
                    {
                        const char* localized = Object::localize("editor/" + lang).c_str();
                        if (ImGui::MenuItem(localized))
                        {
                            Object::language(lang);
                            break;
                        }
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("editor/Material"_localized))
            {
                if (ImGui::MenuItem("editor/Compile"_localized, "editor/Compile current material"_localized, false,
                                    _M_current_material && _M_compiler))
                {
                    _M_shader_compile_error_list.clear();
                    _M_compiler->compile(_M_current_material, _M_shader_compile_error_list);
                }
                ImGui::EndMenu();
            }


            ImGui::EndMenuBar();
        }

        if (_M_frame == 0)
        {
            ImGui::DockBuilderRemoveNode(dock_id);
            ImGui::DockBuilderAddNode(dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);


            auto dock_id_left  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id);
            auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.2f, nullptr, &dock_id);
            auto dock_id_down  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.2f, nullptr, &dock_id);

            ImGui::DockBuilderDockWindow(ImGuiPackageTree::name(), dock_id_left);
            ImGui::DockBuilderDockWindow(ImGuiObjectProperties::name(), dock_id_right);
            ImGui::DockBuilderDockWindow(ImGuiContentBrowser::name(), dock_id_down);
            ImGui::DockBuilderDockWindow("editor/Material Graph###Material Graph"_localized, dock_id);

            ImGui::DockBuilderFinish(dock_id);
        }
    }

    MaterialEditorClient& MaterialEditorClient::update(class RenderViewport* viewport, float dt)
    {
        viewport->window()->imgui_window()->new_frame();
        make_dock_window("MaterialEditorDock", ImGuiWindowFlags_MenuBar, &MaterialEditorClient::render_dock_window, this);
        render_viewport(dt);
        render_material_code();
        viewport->window()->imgui_window()->end_frame();
        ++_M_frame;
        return *this;
    }

    class VisualMaterial* MaterialEditorClient::current_material() const
    {
        return _M_current_material;
    }

    extern void render_material_nodes(class MaterialEditorClient* client);


    static Struct* render_node_types(Group* group)
    {
        if (!group)
            return nullptr;


        Struct* current = nullptr;

        for (Group* child : group->childs())
        {
            if (ImGui::CollapsingHeader(child->name().c_str()))
            {
                ImGui::Indent(10.f);
                Struct* new_struct = render_node_types(child);
                if (!current && new_struct)
                    current = new_struct;

                ImGui::Unindent(10.f);
            }
        }


        for (Struct* instance : group->structs())
        {
            if (ImGui::Selectable(instance->base_name().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (!current && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    current = instance;
                }
            }
        }

        return current;
    }

    static Struct* render_nodes_window()
    {
        static Group* root_group = Group::find("Engine::VisualMaterialNodes");
        return render_node_types(root_group);
    }

    MaterialEditorClient& MaterialEditorClient::render_viewport(float dt)
    {
        ImGui::Begin("editor/Material Graph###Material Graph"_localized);

        if (!ImGui::IsWindowAppearing())
        {
            ax::NodeEditor::SetCurrentEditor(reinterpret_cast<ax::NodeEditor::EditorContext*>(_M_editor_context));

            ax::NodeEditor::Begin("###Viewport");
            if (_M_current_material)
            {
                _M_current_material->render_nodes(_M_editor_context);
            }
            ax::NodeEditor::End();


            if (!_M_current_material)
            {
                ImGui::End();
                return *this;
            }

            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                _M_open_select_node_window = true;
            }


            if (_M_open_select_node_window)
            {
                const char* name = "editor/Menu"_localized;

                ImGui::SetNextWindowSize({300, 350}, ImGuiCond_Appearing);
                ImGui::SetNextWindowSizeConstraints({300, 350}, {FLT_MAX, FLT_MAX});
                auto pos = ImGui::GetMousePos();
                ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);

                ImGui::Begin(name, &_M_open_select_node_window, ImGuiWindowFlags_NoSavedSettings);

                if (ImGui::IsWindowAppearing())
                {
                    _M_next_node_pos = ImGuiHelpers::construct_vec2<Vector2D>(ax::NodeEditor::ScreenToCanvas(pos));
                }

                {
                    Struct* selected = render_nodes_window();

                    if (selected)
                    {
                        _M_open_select_node_window                           = false;
                        _M_current_material->create_node(selected)->position = _M_next_node_pos;
                    }
                }

                ImGui::End();
            }


            ax::NodeEditor::SetCurrentEditor(nullptr);
        }
        ImGui::End();
        return *this;
    }

    void MaterialEditorClient::render_material_code()
    {
        if (_M_open_material_code_window)
        {
            ImGui::SetNextWindowSize({400, 500}, ImGuiCond_Appearing);
            ImGui::Begin("editor/Material Code"_localized, &_M_open_material_code_window);
            ImGui::BeginTabBar("TabBar");

            if (_M_current_material)
            {
                if (ImGui::BeginTabItem("editor/Vertex"_localized))
                {
                    String& code = _M_current_material->pipeline->vertex_shader->text_code;
                    ImGui::TextWrapped("%s", code.c_str());
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("editor/Fragment"_localized))
                {
                    String& code = _M_current_material->pipeline->fragment_shader->text_code;
                    ImGui::TextWrapped("%s", code.c_str());
                    ImGui::EndTabItem();
                }
            }

            if (ImGui::BeginTabItem("editor/Errors"_localized))
            {
                Index index = 1;
                for (auto& msg : _M_shader_compile_error_list)
                {
                    ImGui::TextColored(ImColor(255, 0, 0), "%zu: %s", index++, msg.c_str());
                }
                ImGui::EndTabItem();
            }


            ImGui::EndTabBar();

            ImGui::End();
        }
    }
}// namespace Engine
