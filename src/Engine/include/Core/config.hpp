#pragma once

#include <Core/engine_types.hpp>


namespace Engine
{
    struct ENGINE_EXPORT Config {
        virtual Config& update() = 0;
        virtual ~Config();
    };
}// namespace Engine
