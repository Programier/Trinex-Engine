#pragma once
#include <Core/object.hpp>

namespace Engine
{
    struct PipelineCreateInfo;

    enum class ShaderStage
    {
        Vertex,
        Fragment,
        Geometry,
        Compute,
    };

    class ENGINE_EXPORT ShaderCompiler : public Object
    {
    private:
        static ShaderCompiler* _M_compiler;

    public:
        using Super = Object;

        virtual bool compile(const String& code, ShaderStage stage, Buffer& out_binary);
        virtual bool compile(PipelineCreateInfo* info);
        static ShaderCompiler* load_compiler();
    };
}// namespace Engine
