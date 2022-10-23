#pragma once
#include <panel.hpp>


namespace Engine
{
    class SettingsWindow : public Panel
    {
    private:
        bool _M_first_frame = true;
        std::size_t _M_choiced_index = 0;

    public:
        SettingsWindow();
        void render() override;
    };
}
