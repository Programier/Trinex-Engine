#pragma once

class ImGuiContext;

namespace Engine
{
    void initialize_theme(ImGuiContext* ctx);
    float editor_scale_factor();
}// namespace Engine
