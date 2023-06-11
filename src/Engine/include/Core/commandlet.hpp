#pragma once

#include <Core/object.hpp>


namespace Engine
{
    class ENGINE_EXPORT CommandLet : public Object
    {
    public:
        using Super = Object;

        virtual int execute(int argc, char** argv);
        virtual void on_config_load();
    };
}
