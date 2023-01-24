#include <render_texture.hpp>


namespace Editor
{
    void render_texture(const Engine::Texture2D& texture, const Engine::Size2D& texture_offset, ImVec2* pos,
                        Engine::Size2D* window_size)
    {
        static ImVec2 _pos;
        static Engine::Size2D _size;

        if (!pos)
        {
            _pos = ImGui::GetCursorScreenPos();
            pos = &_pos;
        }

        if (!window_size)
        {
            auto tmp = ImGui::GetWindowSize();
            _size = {tmp.x, tmp.y};
            window_size = &_size;
        }


        auto opengl_texture = texture.internal_id();

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddImage((void*) opengl_texture, *pos, ImVec2(pos->x + window_size->x, pos->y + window_size->y),
                           ImVec2(0, texture_offset.y), ImVec2(texture_offset.x, 0));
    }
}// namespace Editor
