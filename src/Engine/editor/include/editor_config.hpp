#pragma once
#include <Core/config.hpp>

namespace Engine
{
    struct EditorConfig : public Config {
        String material_compiler;
        Path font_path;
        float font_size;
        float collapsing_indent;

        virtual EditorConfig& update();
        virtual EditorConfig& update_using_args();
    };

    extern EditorConfig editor_config;
}// namespace Engine
