#include <Core/garbage_collector.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/material_compiler.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	implement_class_default_init(Engine::MaterialCompiler, 0);

	MaterialCompiler* MaterialCompiler::instance(const StringView& api_name)
	{
		if (api_name.empty())
		{
			StringView new_name = rhi->info.struct_instance->name().to_string();
			if (new_name.empty())
				return nullptr;
			return instance(new_name);
		}

		String full_class_name = Strings::format("Engine::{}_MaterialCompiler", api_name);
		auto* compiler_class   = Refl::Class::static_find(full_class_name);

		if (compiler_class == nullptr)
			return nullptr;

		if (compiler_class->flags(Refl::Struct::IsSingletone))
		{
			if (MaterialCompiler* compiler = instance_cast<MaterialCompiler>(compiler_class->singletone_instance()))
			{
				return compiler;
			}
		}

		Object* compiler_object = compiler_class->create_object();

		if (compiler_object == nullptr)
		{
			return nullptr;
		}

		MaterialCompiler* compiler = instance_cast<MaterialCompiler>(compiler_object);

		if (compiler == nullptr)
		{
			GarbageCollector::destroy(compiler_object);
		}

		return compiler;
	}
}// namespace Engine
