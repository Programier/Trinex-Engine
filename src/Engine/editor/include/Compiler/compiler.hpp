#pragma once
#include <Core/object.hpp>


namespace Engine
{
    class MaterialCompiler : public Object
    {
        declare_class(MaterialCompiler, Object);

    public:
        virtual bool compile(class VisualMaterial* material, MessageList& errors) = 0;
    };
}// namespace Engine
