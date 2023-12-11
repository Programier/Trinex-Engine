#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
#include <Window/window_interface.hpp>
#include <imgui.h>

namespace Engine::ImGuiRenderer
{
    ImDrawData* DrawData::draw_data()
    {
        return &_M_draw_data;
    }

    DrawData& DrawData::release()
    {
        if (_M_draw_data.CmdListsCount > 0)
        {
            for (int index = 0; index < _M_draw_data.CmdListsCount; index++)
            {
                ImDrawList* drawList = _M_draw_data.CmdLists[index];
                delete drawList;
            }

            _M_draw_data.CmdLists.clear();
        }

        _M_draw_data.Clear();
        return *this;
    }

    DrawData& DrawData::copy(ImDrawData* draw_data)
    {
        release();

        _M_draw_data = *draw_data;

        _M_draw_data.CmdLists.resize(draw_data->CmdListsCount);
        for (int index = 0; index < draw_data->CmdListsCount; index++)
        {
            ImDrawList* drawList         = draw_data->CmdLists[index]->CloneOutput();
            _M_draw_data.CmdLists[index] = drawList;
        }

        return *this;
    }

    DrawData::~DrawData()
    {
        release();
    }

    Window::Window(WindowInterface* interface, ImGuiContext* ctx) : _M_context(ctx), _M_interface(interface)
    {}

    ImGuiContext* Window::context() const
    {
        return _M_context;
    }

    ImDrawData* Window::draw_data()
    {
        return _M_draw_data.draw_data();
    }

    Window& Window::new_frame()
    {
        ImGui::SetCurrentContext(_M_context);
        _M_interface->new_imgui_frame();

        RHI* rhi = engine_instance->rhi();
        rhi->imgui_new_frame(_M_context);
        ImGui::NewFrame();

        return *this;
    }

    Window& Window::end_frame()
    {
        ImGui::SetCurrentContext(_M_context);
        ImGui::Render();
        return *this;
    }

    Window& Window::prepare_render()
    {
        ImGui::SetCurrentContext(_M_context);
        _M_draw_data.copy(ImGui::GetDrawData());
        return *this;
    }

    Window& Window::render()
    {
        RHI* rhi = engine_instance->rhi();
        rhi->imgui_render(_M_context, draw_data());
        return *this;
    }

    Window::~Window()
    {}
}// namespace Engine::ImGuiRenderer
