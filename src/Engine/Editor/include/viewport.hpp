#pragma once
#include <panel.hpp>
#include <Graphics/framebuffer.hpp>

namespace Editor
{
    class ViewPort : public Panel
    {
        Engine::FrameBuffer _M_framebuffer;
        Engine::Texture2D _M_depth_texture;
        Engine::Texture2D _M_colored_texture;
        Engine::Texture2D _M_scentil_texture;
        bool _M_hidden_cursor = false;
        bool _M_camera_mode = false;
        Engine::Size2D _M_viewport_size;
        bool _M_octree_render = false;

        void move_camera();
    public:
        ViewPort();
        void render() override;
        void proccess_commands() override;
    };
}
