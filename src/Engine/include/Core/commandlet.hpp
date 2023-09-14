#pragma once

#include <Core/object.hpp>


namespace Engine
{
    class ENGINE_EXPORT CommandLet : public Object
    {
        declare_class(CommandLet, Object);

    public:
        virtual int_t execute(int_t argc, char** argv);
        virtual CommandLet& load_configs();
    };
}// namespace Engine
