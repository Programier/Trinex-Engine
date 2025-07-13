#include <Core/garbage_collector.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_compiler.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	trinex_implement_class_default_init(Engine::ShaderCompiler, 0);

	bool ShaderCompilationResult::initialize_pipeline(class GraphicsPipeline* pipeline)
	{
		if (shaders.vertex.empty() || shaders.fragment.empty())
			return false;

		pipeline->clear();
		pipeline->vertex_shader(true)->source_code   = shaders.vertex;
		pipeline->vertex_shader(true)->attributes    = reflection.vertex_attributes;
		pipeline->fragment_shader(true)->source_code = shaders.fragment;
		pipeline->parameters(reflection.parameters);
		
		if (!shaders.tessellation_control.empty())
			pipeline->tessellation_control_shader(true)->source_code = shaders.tessellation_control;
		if (!shaders.tessellation.empty())
			pipeline->tessellation_shader(true)->source_code = shaders.tessellation;
		if (!shaders.geometry.empty())
			pipeline->geometry_shader(true)->source_code = shaders.geometry;
		return true;
	}

	bool ShaderCompilationResult::initialize_pipeline(class ComputePipeline* pipeline)
	{
		if (shaders.compute.empty())
			return false;

		pipeline->clear();
		pipeline->compute_shader()->source_code = shaders.compute;
		pipeline->parameters(reflection.parameters);
		return true;
	}

	ShaderCompiler* ShaderCompiler::instance(const StringView& api_name)
	{
		if (api_name.empty())
		{
			StringView new_name = rhi->info.struct_instance->name().to_string();
			if (new_name.empty())
				return nullptr;
			return instance(new_name);
		}

		String full_class_name = Strings::format("Engine::{}_ShaderCompiler", api_name);
		auto* compiler_class   = Refl::Class::static_find(full_class_name);

		if (compiler_class == nullptr)
			return nullptr;

		if (compiler_class->flags(Refl::Struct::IsSingletone))
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
}// namespace Engine
