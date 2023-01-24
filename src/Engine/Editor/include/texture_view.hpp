#pragma once

#include <panel.hpp>
#include <Graphics/texture_2D.hpp>



namespace  Editor
{
    class TextureView : public Panel
    {
        Engine::Texture2D _M_texture;
        Panel* _M_parent_panel = nullptr;
        std::string _M_title;
        bool _M_is_open = true;
        bool _M_init_frame = true;

    public:
        TextureView(const Engine::Texture2D& texture, Panel* panel,  const std::string& title);
        void render() override;

    };
}
