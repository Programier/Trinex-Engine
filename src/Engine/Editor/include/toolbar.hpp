#pragma once
#include <panel.hpp>
#include <vector>

namespace Engine
{
    class ToolBar : public Panel
    {
    private:
        std::vector<Panel*> _M_render_panel;

    public:
        ToolBar();
        void render() override;
        ~ToolBar();
    };
}
