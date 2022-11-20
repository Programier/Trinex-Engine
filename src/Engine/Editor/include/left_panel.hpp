#pragma once
#include <panel.hpp>

namespace Editor
{
    class LeftPanel : public Panel
    {
    private:
        void process_keyboard();
    public:
        void render() override;
    };
}
