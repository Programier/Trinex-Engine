#pragma once

#include <Core/engine_types.hpp>


namespace Engine
{
    class Config
    {
    protected:
        Function<void()> _M_callback;

        virtual void write_lua_object(void* object) = 0;

    public:
        Config& load_config(const String& filename);
        Config& save_config(const String& filename);
        virtual ~Config();
    };
}
