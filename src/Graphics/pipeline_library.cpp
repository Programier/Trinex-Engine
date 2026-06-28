#include <Core/file_manager.hpp>
#include <Core/garbage_collector.hpp>
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
		auto& manifest = PipelineLibraryCacheManifest::instance();
		const Name permutation;

		const PipelineLibraryCacheIndexEntry* index = manifest.find(full_name(), permutation);

		if (index == nullptr || !cache.load_by_hash(index->shader_hash))
		{
			return false;
		}

		release_childs();

		Pipeline* default_pipeline = create_pipeline_instance(cache.type, permutation);

		if (default_pipeline == nullptr)
		{
			trinex_error(Log::Graphics, "Failed to create default pipeline instance for '%s'", full_name().c_str());
			return false;
		}

		cache.apply_to(default_pipeline);
		default_pipeline->rebuild();
		return true;
	}

	GlobalPipelineLibrary& GlobalPipelineLibrary::load_pipeline()
	{
		ShaderCompiler* compiler = ShaderCompiler::instance();
		bool loaded              = load_default_pipeline_cache();

		if (!loaded && compiler)
		{
			loaded = compile(compiler);
		}

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
		auto& manifest     = PipelineLibraryCacheManifest::instance();
		Pipeline* pipeline = nullptr;

		if (!cache.load_by_hash(result.shader_hash))
		{
			cache.init_from(result);

			if (!cache.store_by_hash(result.shader_hash))
			{
				String message = Strings::format("{:016x}{:016x}", static_cast<u64>(result.shader_hash >> 64),
				                                 static_cast<u64>(result.shader_hash));
				trinex_warning(Log::Graphics, "Failed to store shader cache for hash '%s'", message.c_str());
			}
		}

		auto& index       = manifest.entry(full_name(), result.permutation);
		index.type        = result.shaders.compute.empty() ? PipelineLibraryCache::Graphics : PipelineLibraryCache::Compute;
		index.shader_hash = result.shader_hash;

		pipeline = create_pipeline_instance(cache.type, result.permutation);

		if (pipeline == nullptr)
		{
			trinex_error(Log::Graphics, "Failed to create pipeline instance for permutation '%s'", result.permutation.c_str());
			return false;
		}

		cache.apply_to(pipeline);
		pipeline->rebuild();
		return true;
	}

	bool GlobalPipelineLibrary::compile(ShaderCompiler* compiler)
	{
		if (compiler == nullptr)
		{
			compiler = ShaderCompiler::instance();

			if (compiler == nullptr)
			{
				trinex_error(Log::Graphics, "Failed to find shader compiler");
				return false;
			}
		}

		const char* path = source_path();

		FileReader reader(path);

		if (!reader.is_open())
		{
			trinex_error(Log::Graphics, "Failed to open shader module '%s'", path);
			return false;
		}

		const String source = reader.read_string();

		if (source.empty())
		{
			trinex_error(Log::Graphics, "Shader module '%s' is empty", path);
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
