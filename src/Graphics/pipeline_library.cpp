#include <Core/file_manager.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_library.hpp>
#include <Graphics/shader_cache.hpp>
#include <Graphics/shader_compiler.hpp>

namespace Trinex
{
	trinex_implement_engine_class_default_init(PipelineLibrary, 0);
	trinex_implement_engine_class_default_init(GlobalPipelineLibrary, 0);

	Pipeline* PipelineLibrary::create_pipeline_instance(u8 type, Name name)
	{
		Pipeline* pipeline = nullptr;

		switch (type)
		{
			case PipelineLibraryCache::Graphics: pipeline = Object::new_instance<GraphicsPipeline>(name, this); break;
			case PipelineLibraryCache::Compute: pipeline = Object::new_instance<ComputePipeline>(name, this); break;
			default: break;
		}

		return pipeline;
	}

	String PipelineLibrary::permutation_cache_name(Name name) const
	{
		if (name == Name::none || !name.is_valid())
		{
			return full_name();
		}

		return Strings::format("{}::{}", full_name(), name.to_string());
	}

	Pipeline* PipelineLibrary::find_pipeline(const Name& key) const
	{
		return instance_cast<Pipeline>(ObjectTreeNode::find_child_object(key));
	}

	GraphicsPipeline* PipelineLibrary::find_graphics_pipeline(const Name& key) const
	{
		return instance_cast<GraphicsPipeline>(ObjectTreeNode::find_child_object(key));
	}

	ComputePipeline* PipelineLibrary::find_compute_pipeline(const Name& key) const
	{
		return instance_cast<ComputePipeline>(ObjectTreeNode::find_child_object(key));
	}

	StringView GlobalPipelineLibrary::pipeline_name_of(StringView name)
	{
		return Strings::class_name_sv_of(name);
	}

	Object* GlobalPipelineLibrary::package_of(StringView name)
	{
		Package* package         = Package::static_find_package("TrinexEngine::GlobalPipelines", true);
		StringView child_package = Strings::namespace_sv_of(name);

		if (!child_package.empty())
		{
			package = package->find_package(child_package, true);
		}

		return package;
	}

	GlobalPipelineLibrary::GlobalPipelineLibrary(StringView name) {}

	bool GlobalPipelineLibrary::load_default_pipeline_cache()
	{
		PipelineLibraryCache cache;
		const Name permutation;

		if (!cache.load(permutation_cache_name(permutation)))
		{
			return false;
		}

		release_childs();

		Pipeline* default_pipeline = create_pipeline_instance(cache.type, permutation);

		if (default_pipeline == nullptr)
		{
			error_log("GlobalPipelineLibrary", "Failed to create default pipeline instance for '%s'", full_name().c_str());
			return false;
		}

		cache.apply_to(default_pipeline);
		default_pipeline->init_render_resources();
		return true;
	}

	GlobalPipelineLibrary& GlobalPipelineLibrary::load_pipeline()
	{
		ShaderCompiler* compiler = ShaderCompiler::instance();
		const bool loaded        = compiler ? compile(compiler) : load_default_pipeline_cache();

		trinex_verify_msg(loaded, "Failed to load global pipeline library");

		initialize();
		return *this;
	}

	GlobalPipelineLibrary& GlobalPipelineLibrary::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		return *this;
	}

	bool GlobalPipelineLibrary::compile_permutation(const ShaderCompilationResult& result)
	{
		PipelineLibraryCache cache;
		const String cache_name = permutation_cache_name(result.permutation);
		Pipeline* pipeline      = nullptr;

		if (!cache.load(cache_name))
		{
			cache.init_from(result);

			if (!cache.store(cache_name))
			{
				warn_log("GlobalPipelineLibrary", "Failed to store shader cache for '%s'", cache_name.c_str());
			}
		}

		pipeline = create_pipeline_instance(cache.type, result.permutation);

		if (pipeline == nullptr)
		{
			error_log("GlobalPipelineLibrary", "Failed to create pipeline instance for permutation '%s'",
			          result.permutation.c_str());
			return false;
		}

		cache.apply_to(pipeline);
		pipeline->init_render_resources();
		return true;
	}

	bool GlobalPipelineLibrary::compile(ShaderCompiler* compiler)
	{
		if (compiler == nullptr)
		{
			compiler = ShaderCompiler::instance();

			if (compiler == nullptr)
			{
				error_log("PipelineLibrary", "Failed to find shader compiler");
				return false;
			}
		}

		const char* path = source_path();

		FileReader reader(path);

		if (!reader.is_open())
		{
			error_log("PipelineLibrary", "Failed to open shader module '%s'", path);
			return false;
		}

		const String source = reader.read_string();

		if (source.empty())
		{
			error_log("PipelineLibrary", "Shader module '%s' is empty", path);
			return false;
		}

		release_childs();

		ShaderCompiler::StackEnvironment env;
		env.add_source(source.c_str());
		modify_compilation_env(&env);

		return compiler->compile(&env, [this](const ShaderCompilationResult& result) { return compile_permutation(result); });
	}

	Pipeline* GlobalPipelineLibrary::pipeline(Name permutation) const
	{
		return find_pipeline(permutation);
	}

	GraphicsPipeline* GlobalPipelineLibrary::graphics_pipeline(Name permutation) const
	{
		return find_graphics_pipeline(permutation);
	}

	ComputePipeline* GlobalPipelineLibrary::compute_pipeline(Name permutation) const
	{
		return find_compute_pipeline(permutation);
	}

	RHIPipeline* GlobalPipelineLibrary::handle(Name permutation) const
	{
		Pipeline* result = pipeline(permutation);
		return result ? result->handle() : nullptr;
	}

	const RHIShaderParameterInfo* GlobalPipelineLibrary::find_parameter(const Name& key, Name permutation) const
	{
		Pipeline* result = pipeline(permutation);
		return result ? result->find_parameter(key) : nullptr;
	}
}// namespace Trinex
