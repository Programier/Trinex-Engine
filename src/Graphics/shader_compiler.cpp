#include <Core/class.hpp>
#include <Core/garbage_collector.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader_compiler.hpp>

namespace Engine::ShaderCompiler
{
    implement_class_default_init(Engine::ShaderCompiler, Compiler, 0);

    Compiler* Compiler::static_create_compiler(const StringView& api_name)
    {
        if (api_name.empty())
        {
            StringView new_name = rhi->info.struct_instance->base_name().to_string();
            if (new_name.empty())
                return nullptr;
            return static_create_compiler(new_name);
        }

        String full_class_name = Strings::format("Engine::ShaderCompiler::{}_Compiler", api_name);
        Class* compiler_class  = Class::static_find(full_class_name);
        if (compiler_class == nullptr)
            return nullptr;
        Object* compiler_object = compiler_class->create_object();

        if (compiler_object == nullptr)
        {
            return nullptr;
        }

        Compiler* compiler = compiler_object->instance_cast<Compiler>();
        if (compiler == nullptr)
        {
            GarbageCollector::destroy(compiler_object);
        }
        return compiler;
    }
}// namespace Engine::ShaderCompiler
