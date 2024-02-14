#pragma once
#include <Core/object.hpp>


namespace Engine
{
    class MaterialNode;
    class MaterialPin;
    class MaterialInputPin;
    class MaterialOutputPin;

    class ShaderCompiler : public Object
    {
        declare_class(ShaderCompiler, Object);

    public:
        virtual bool compile(class VisualMaterial* material, MessageList& errors) = 0;

        virtual size_t sin(MaterialInputPin*)      = 0;
        virtual size_t float_constant(float value) = 0;

        virtual size_t base_color(MaterialInputPin*) = 0;
    };
}// namespace Engine
