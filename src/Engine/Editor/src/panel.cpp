#include <Event/mouse_event.hpp>
#include <ImGui/imgui.h>
#include <panel.hpp>

namespace Editor
{
    Panel::~Panel() = default;
    bool Panel::cursor_on_panel() const
    {
        ImVec2 viewport_size = ImGui::GetWindowSize();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        static const auto& mouse_pos = Engine::MouseEvent::position();

        if (mouse_pos.x - pos.x < viewport_size.x && mouse_pos.y - pos.y < viewport_size.y && mouse_pos.x > pos.x &&
            mouse_pos.y > pos.y)
        {
            return true;
        }
        return false;
    }

    void Panel::proccess_commands()
    {}


    void Panel::set_panel(Panel* panel)
    {
        _M_windows.insert(panel);
    }

    void Panel::remove_panel(Panel* panel)
    {
        _M_windows.erase(panel);
    }
}// namespace Editor
