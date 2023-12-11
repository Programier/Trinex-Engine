#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
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
}// namespace Engine::ImGuiRenderer
