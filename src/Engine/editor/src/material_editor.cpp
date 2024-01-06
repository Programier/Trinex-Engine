#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/package.hpp>
#include <Core/thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
#include <dock_window.hpp>
#include <imgui_internal.h>
#include <imgui_windows.hpp>
#include <material_editor_client.hpp>
#include <theme.hpp>


#define MATERIAL_EDITOR_DEBUG 1

namespace Engine
{
    implement_engine_class_default_init(MaterialEditorClient);


    static bool is_material(Object* object)
    {
        const class Class* class_instance = object->class_instance();
        if (class_instance->contains_class(MaterialInterface::static_class_instance()))
            return true;

        if (class_instance == MaterialObject::static_class_instance())
            return true;

        return false;
    }

    MaterialEditorClient& MaterialEditorClient::on_bind_to_viewport(class RenderViewport* viewport)
    {
        Window* window = viewport->window();
        if (window == nullptr)
        {
            throw EngineException("Cannot bind client to non-window viewport!");
        }

        window->imgui_initialize(initialize_theme);
        String new_title = window->title() + Strings::format(" [{} RHI]", engine_instance->rhi()->name().c_str());
        window->title(new_title);

        engine_instance->thread(ThreadType::RenderThread)->wait_all();
        _M_viewport = viewport;

        _M_package_tree.on_package_select.push(std::bind(&MaterialEditorClient::on_package_select, this, std::placeholders::_1));
        _M_content_browser.filters.push(is_material);

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
        _M_content_browser.package = package;
    }

    void MaterialEditorClient::render_dock_window(void* userdata)
    {
        auto dock_id                       = ImGui::GetID("MaterialEditorDock##Dock");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        if (_M_frame == 0)
        {
            ImGui::DockBuilderRemoveNode(dock_id);
            ImGui::DockBuilderAddNode(dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);


            auto dock_id_left = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.2f, nullptr, &dock_id);

            ImGui::DockBuilderDockWindow(_M_package_tree.name(), dock_id_left);
            ImGui::DockBuilderDockWindow(_M_content_browser.name(), dock_id_down);

            ImGui::DockBuilderFinish(dock_id);
        }
    }

    MaterialEditorClient& MaterialEditorClient::update(class RenderViewport* viewport, float dt)
    {
        viewport->window()->imgui_window()->new_frame();
        make_dock_window("MaterialEditorDock", 0, &MaterialEditorClient::render_dock_window, this);
        render_viewport(dt).render_properties(dt);

        _M_package_tree.render(viewport);
        _M_content_browser.render(viewport);

        viewport->window()->imgui_window()->end_frame();
        ++_M_frame;
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::prepare_render(class RenderViewport* viewport)
    {
        viewport->window()->imgui_window()->prepare_render();
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render_viewport(float dt)
    {
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render_properties(float dt)
    {
        ImGui::Begin("Properties");


        ImGui::End();
        return *this;
    }
}// namespace Engine
