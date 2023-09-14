#pragma once
#include <Core/object.hpp>

namespace Engine
{
    struct PipelineCreateInfo;

    enum class ShaderStage
    {
        Vertex   = 0,
        Fragment = 1,
        Geometry = 2,
        Compute  = 3,
    };

    // Shader compiler require
    class ENGINE_EXPORT ShaderCompiler : public Object
    {
        declare_class(ShaderCompiler, Object);

    public:
        using ErrorList = List<String>;

    private:
        static ShaderCompiler* _M_compiler;

    public:
        virtual bool compile(const String& code, ShaderStage stage, Buffer& out_binary, bool debug = true,
                             ErrorList* errors = nullptr);
        virtual bool compile(PipelineCreateInfo* info, bool debug = true, ErrorList* errors = nullptr);
        virtual bool update_reflection(PipelineCreateInfo* info);
        static ShaderCompiler* load_compiler();
    };
}// namespace Engine
