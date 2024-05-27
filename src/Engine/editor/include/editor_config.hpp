#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    struct EditorConfig {
        Path font_path;
        float font_size;
        float collapsing_indent;

        EditorConfig& update();
    };

    extern EditorConfig editor_config;
}// namespace Engine
