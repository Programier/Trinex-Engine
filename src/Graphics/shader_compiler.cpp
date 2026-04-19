#include <Core/garbage_collector.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_compiler.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	trinex_implement_class_default_init(Trinex::ShaderCompiler, 0);

	bool ShaderCompilationResult::initialize_pipeline(class GraphicsPipeline* pipeline)
	{
		if (shaders.vertex.empty())
			return false;

		pipeline->clear();
		pipeline->vertex_attributes             = reflection.vertex_attributes;
		pipeline->vertex_shader(true)->source   = shaders.vertex;
		pipeline->fragment_shader(true)->source = shaders.fragment;
		pipeline->parameters(reflection.parameters);

		if (!shaders.tessellation_control.empty())
			pipeline->tessellation_control_shader(true)->source = shaders.tessellation_control;
		if (!shaders.tessellation.empty())
			pipeline->tessellation_shader(true)->source = shaders.tessellation;
		if (!shaders.geometry.empty())
			pipeline->geometry_shader(true)->source = shaders.geometry;
		return true;
	}

	bool ShaderCompilationResult::initialize_pipeline(class ComputePipeline* pipeline)
	{
		if (shaders.compute.empty())
			return false;

		pipeline->clear();
		pipeline->compute_shader()->source = shaders.compute;
		pipeline->parameters(reflection.parameters);
		return true;
	}

	ShaderCompiler* ShaderCompiler::instance(const StringView& api_name)
	{
		if (api_name.empty())
		{
			StringView new_name = RHI::instance()->info.struct_instance->name().to_string();
			if (new_name.empty())
				return nullptr;
			return instance(new_name);
		}

		String full_class_name = Strings::format("Trinex::{}_ShaderCompiler", api_name);
		auto* compiler_class   = Refl::Class::static_find(full_class_name);

		if (compiler_class == nullptr)
			return nullptr;

		if (compiler_class->flags.any(Refl::Struct::IsSingletone))
		{
			if (ShaderCompiler* compiler = instance_cast<ShaderCompiler>(compiler_class->singletone_instance()))
			{
				return compiler;
			}
		}

		Object* compiler_object = compiler_class->create_object();

		if (compiler_object == nullptr)
		{
			return nullptr;
		}

		ShaderCompiler* compiler = instance_cast<ShaderCompiler>(compiler_object);

		if (compiler == nullptr)
		{
			GarbageCollector::destroy(compiler_object);
		}

		return compiler;
	}
}// namespace Trinex
