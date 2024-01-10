#pragma once

#include <Core/engine_types.hpp>


namespace Engine
{
    struct ENGINE_EXPORT Config {
        virtual Config& update();
        virtual Config& update_using_args();
        virtual ~Config();
    };
}// namespace Engine
