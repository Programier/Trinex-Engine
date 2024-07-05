#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT SplashConfig {
        Path image_path;
        Path font_path;
        int_t startup_text_size;
        int_t version_text_size;
        int_t copyright_text_size;
        int_t game_name_text_size;


        SplashConfig();
        SplashConfig& init();
    };
}// namespace Engine
