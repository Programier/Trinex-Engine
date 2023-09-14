#include <Core/class.hpp>
#include <Core/engine_config.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/shader_compiler.hpp>

namespace Engine
{
    ShaderCompiler* ShaderCompiler::_M_compiler;

    bool ShaderCompiler::compile(const String& code, ShaderStage stage, Buffer& out_binary, bool debug,
                                 ErrorList* errors)
    {
        error_log("ShaderCompiler", "Method '%s' is not implemented!", __PRETTY_FUNCTION__);
        return false;
    }

    bool ShaderCompiler::compile(PipelineCreateInfo* info, bool debug, ErrorList* errors)
    {
        error_log("ShaderCompiler", "Method '%s' is not implemented!", __PRETTY_FUNCTION__);
        return false;
    }

    bool ShaderCompiler::update_reflection(PipelineCreateInfo* info)
    {
        error_log("ShaderCompiler", "Method '%s' is not implemented!", __PRETTY_FUNCTION__);
        return false;
    }

    ShaderCompiler* ShaderCompiler::load_compiler()
    {
        if (_M_compiler == nullptr)
        {
            info_log("ShaderCompiler", "Loading library: '%s'", engine_config.shader_compilers_lib.c_str());
            Library lib(engine_config.shader_compilers_lib);
            if (!lib.has_lib())
            {
                error_log("ShaderCompiler", "Failed to load library '%s'", engine_config.shader_compilers_lib.c_str());
                return nullptr;
            }

            Class* compiler_class = Class::static_find_class(engine_config.shader_compiler);
            if (compiler_class == nullptr)
            {
                error_log("ShaderCompiler", "Failed to find shader compiler class '%'",
                          engine_config.shader_compiler.c_str());
                return nullptr;
            }

            if (!compiler_class->contains_class(Class::static_find_class("Engine::ShaderCompiler")))
            {
                error_log("ShaderCompiler", "Class '%s' does not inherit from class Engine::ShaderCompiler!",
                          compiler_class->name().c_str());
                return nullptr;
            }

            _M_compiler = compiler_class->create_object()->instance_cast<ShaderCompiler>();
        }

        return _M_compiler;
    }

    implement_class(ShaderCompiler, "Engine");
    implement_default_initialize_class(ShaderCompiler);
}// namespace Engine
