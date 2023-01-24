#include <render_texture.hpp>
#include <texture_view.hpp>


namespace Editor
{
    TextureView::TextureView(const Engine::Texture2D& texture, Panel* panel, const std::string& title)
    {
        _M_texture = texture;
        _M_parent_panel = panel;
        _M_title = title;
        panel->set_panel(this);
    }

    void TextureView::render()
    {
        if (_M_init_frame)
        {
            ImGui::SetNextWindowSize({400, 300});
            _M_init_frame = false;
        }
        if (ImGui::Begin(_M_title.c_str(), &_M_is_open))
        {
            render_texture(_M_texture);
        }

        ImGui::End();

        if (!_M_is_open)
        {
            _M_parent_panel->remove_panel(this);
            delete this;
        }
    }
}// namespace Editor
