#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/package.hpp>
#include <Core/thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
#include <dock_window.hpp>
#include <material_editor_client.hpp>
#include <theme.hpp>
#include <Graphics/material.hpp>

namespace Engine
{
    implement_engine_class_default_init(MaterialEditorClient);


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
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::render(class RenderViewport* viewport)
    {
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::update(class RenderViewport* viewport, float dt)
    {
        viewport->window()->imgui_window()->new_frame();
        make_dock_window("MaterialEditorDock");
        render_properties(dt);
        viewport->window()->imgui_window()->end_frame();
        return *this;
    }

    MaterialEditorClient& MaterialEditorClient::prepare_render(class RenderViewport* viewport)
    {
        viewport->window()->imgui_window()->prepare_render();
        return *this;
    }


    static void render_material_tree(Package* package)
    {
        if (ImGui::CollapsingHeader(package->string_name().c_str()))
        {
            ImGui::Indent(10.f);

            for (auto& [name, object] : package->objects())
            {
            }

            for (auto& [name, object] : package->objects())
            {
                Package* next_package = object->instance_cast<Package>();
                if(next_package)
                {
                    render_material_tree(next_package);
                }
            }

            ImGui::Unindent(10.0f);
        }
    }

    MaterialEditorClient& MaterialEditorClient::render_properties(float dt)
    {
        Package* package = Object::root_package();
        ImGui::Begin("Resource Tree");
        render_material_tree(package);
        ImGui::End();

        return *this;
    }
}// namespace Engine
