#pragma once

#include <Core/object.hpp>


namespace Engine
{
    class ENGINE_EXPORT CommandLet : public Object
    {
    public:
        using Super = Object;

        virtual int_t execute(int_t argc, char** argv);
        virtual void on_config_load();
    };
}
