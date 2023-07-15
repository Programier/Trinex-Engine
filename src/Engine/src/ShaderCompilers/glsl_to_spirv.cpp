#include <Core/class.hpp>
#include <Core/shader_compiler.hpp>

namespace Engine
{
    class GLSLToSPIRV : public ShaderCompiler
    {
    public:
        using Super = ShaderCompiler;

        bool compile(const String& code, ShaderStage stage, Buffer& out_binary) override
        {
            return false;
        }

        bool process_reflection(PipelineCreateInfo* info) override
        {
            return false;
        }
    };

    register_class(Engine::GLSLToSPIRV);
}// namespace Engine
