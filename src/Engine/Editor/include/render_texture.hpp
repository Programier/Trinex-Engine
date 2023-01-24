#pragma once
#include <Core/engine_types.hpp>
#include <ImGui/imgui.h>
#include <Graphics/texture_2D.hpp>

namespace Editor
{
    void render_texture(const Engine::Texture2D& texture, const Engine::Size2D& texture_offset = {1, 1},
                        ImVec2* pos = nullptr, Engine::Size2D* window_size = nullptr);
}
