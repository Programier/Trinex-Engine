#pragma once

#include <panel.hpp>

namespace Editor
{
    class MenuBar : public Panel
    {
        void file_button();
    public:
        void render() override;
    };
}// namespace Editor
