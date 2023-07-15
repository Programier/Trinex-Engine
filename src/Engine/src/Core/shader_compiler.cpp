#include <Core/class.hpp>
#include <Core/engine_config.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/shader_compiler.hpp>

namespace Engine
{
    ShaderCompiler* ShaderCompiler::_M_compiler;

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

            Class* compiler_class = Class::find_class(engine_config.shader_compiler);
            if (compiler_class == nullptr)
            {
                error_log("ShaderCompiler", "Failed to find shader compiler class '%'",
                          engine_config.shader_compiler.c_str());
                return nullptr;
            }

            if (!compiler_class->contains_class(Class::find_class("Engine::ShaderCompiler")))
            {
                error_log("ShaderCompiler", "Class '%s' does not inherit from class Engine::ShaderCompiler!",
                          compiler_class->name().c_str());
                return nullptr;
            }

            _M_compiler = compiler_class->create()->instance_cast<ShaderCompiler>();
        }

        return _M_compiler;
    }

    register_class(Engine::ShaderCompiler);
}// namespace Engine
