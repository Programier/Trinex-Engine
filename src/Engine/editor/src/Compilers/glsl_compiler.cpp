#include <Compiler/compiler.hpp>
#include <Graphics/visual_material.hpp>


namespace Engine
{
    class GLSL_Compiler : public ShaderCompiler
    {
    public:
        bool compile(class VisualMaterial* material, MessageList& errors) override
        {
            return false;
        }

        size_t sin(MaterialInputPin*) override
        {
            return MaterialNode::compile_error;
        }
    };
}// namespace Engine
