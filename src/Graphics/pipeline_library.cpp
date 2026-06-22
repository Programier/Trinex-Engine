#include <Core/file_manager.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_library.hpp>
#include <Graphics/shader_cache.hpp>
#include <Graphics/shader_compiler.hpp>

namespace Trinex
{
	trinex_implement_engine_class_default_init(PipelineLibrary, 0);

	PipelineLibrary::PipelineLibrary()
	{}

	PipelineLibrary::~PipelineLibrary()
	{
		clear_pipelines();
	}

	void PipelineLibrary::clear_pipelines()
	{
		for (auto& [name, pipeline] : m_pipelines)
		{
			if (pipeline)
			{
				pipeline->owner(nullptr);
			}
		}

		m_pipelines.clear();
	}

	PipelineLibrary& PipelineLibrary::shader_path(const Path& path)
	{
		m_shader_path = path;
		return *this;
	}

	const Path& PipelineLibrary::shader_path() const
	{
		return m_shader_path;
	}

	const Vector<ShaderPermutationDescriptor>& PipelineLibrary::permutations() const
	{
		return m_permutations;
	}

	const PipelineLibrary::Pipelines& PipelineLibrary::pipelines() const
	{
		return m_pipelines;
	}

	PipelineLibrary& PipelineLibrary::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		return *this;
	}

	Pipeline* PipelineLibrary::create_pipeline_instance(u8 type, const ShaderPermutationDescriptor& permutation)
	{
		Pipeline* pipeline = nullptr;
		const String name  = permutation.canonical_name();

		switch (type)
		{
			case PipelineLibraryCache::Graphics: pipeline = Object::new_instance<GraphicsPipeline>(name, this); break;
			case PipelineLibraryCache::Compute: pipeline = Object::new_instance<ComputePipeline>(name, this); break;
			default: break;
		}

		return pipeline;
	}

	PipelineLibrary& PipelineLibrary::initialize_pipeline(Pipeline* pipeline, const ShaderPermutationDescriptor& permutation)
	{
		return *this;
	}

	String PipelineLibrary::permutation_cache_name(const ShaderPermutationDescriptor& permutation) const
	{
		return Strings::format("{}::{}", full_name(), permutation.canonical_name());
	}

	bool PipelineLibrary::compile_permutation(const ShaderCompilationResult& result)
	{
		const ShaderPermutationDescriptor& permutation = result.permutation;
		PipelineLibraryCache cache;
		const String cache_name = permutation_cache_name(permutation);
		Pipeline* pipeline      = nullptr;

		if (!cache.load(cache_name))
		{
			cache.init_from(result);

			if (!cache.store(cache_name))
			{
				warn_log("PipelineLibrary", "Failed to store shader cache for '%s'", cache_name.c_str());
			}
		}

		pipeline = create_pipeline_instance(cache.type, permutation);

		if (pipeline == nullptr)
		{
			error_log("PipelineLibrary", "Failed to create pipeline instance for permutation '%s'",
			          permutation.canonical_name().c_str());
			return false;
		}

		cache.apply_to(pipeline);
		initialize_pipeline(pipeline, permutation);
		pipeline->init_render_resources();
		m_pipelines[permutation.canonical_name()] = pipeline;
		return true;
	}

	bool PipelineLibrary::compile(ShaderCompiler* compiler)
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

		FileReader reader(m_shader_path);

		if (!reader.is_open())
		{
			error_log("PipelineLibrary", "Failed to open shader module '%s'", m_shader_path.c_str());
			return false;
		}

		const String source = reader.read_string();

		if (source.empty())
		{
			error_log("PipelineLibrary", "Shader module '%s' is empty", m_shader_path.c_str());
			return false;
		}

		clear_pipelines();
		m_permutations.clear();

		ShaderCompiler::StackEnvironment env;
		env.add_source(source.c_str());
		modify_compilation_env(&env);

		return compiler->compile(&env, [&](const ShaderCompilationResult& result) {
			m_permutations.push_back(result.permutation);
			return compile_permutation(result);
		});
	}

	Pipeline* PipelineLibrary::find_pipeline(const ShaderPermutationKey& key) const
	{
		const String name = key.canonical_name();
		auto it           = m_pipelines.find(name);
		return it == m_pipelines.end() ? nullptr : it->second;
	}

	GraphicsPipeline* PipelineLibrary::find_graphics_pipeline(const ShaderPermutationKey& key) const
	{
		return Object::instance_cast<GraphicsPipeline>(find_pipeline(key));
	}

	ComputePipeline* PipelineLibrary::find_compute_pipeline(const ShaderPermutationKey& key) const
	{
		return Object::instance_cast<ComputePipeline>(find_pipeline(key));
	}

	PipelineLibrary& PipelineLibrary::release_render_resources()
	{
		Super::release_render_resources();

		for (auto& [name, pipeline] : m_pipelines)
		{
			if (pipeline)
				pipeline->release_render_resources();
		}

		return *this;
	}
}// namespace Trinex
