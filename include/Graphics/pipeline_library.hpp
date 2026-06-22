#pragma once
#include <Core/etl/map.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/pointer.hpp>
#include <Graphics/render_resource.hpp>

namespace Trinex
{
	class Pipeline;
	class GraphicsPipeline;
	class ComputePipeline;
	class ShaderCompiler;
	class ShaderCompilationEnvironment;
	class ShaderCompilationResult;
	struct PipelineLibraryCache;

	class ENGINE_EXPORT PipelineLibrary : public RenderResource
	{
		trinex_class(PipelineLibrary, RenderResource);

	public:
		using Pipelines = TreeMap<Name, Pointer<Pipeline>>;

	private:
		Path m_shader_path;
		Pipelines m_pipelines;

		bool compile_permutation(const ShaderCompilationResult& result);
		void clear_pipelines();

	protected:
		virtual PipelineLibrary& modify_compilation_env(ShaderCompilationEnvironment* env);
		virtual Pipeline* create_pipeline_instance(u8 type, Name name);
		virtual PipelineLibrary& initialize_pipeline(Pipeline* pipeline, Name name);
		virtual String permutation_cache_name(Name name) const;

	public:
		PipelineLibrary();
		~PipelineLibrary();

		PipelineLibrary& shader_path(const Path& path);
		const Path& shader_path() const;
		const Pipelines& pipelines() const;

		bool compile(ShaderCompiler* compiler = nullptr);
		Pipeline* find_pipeline(const Name& key) const;
		GraphicsPipeline* find_graphics_pipeline(const Name& key) const;
		ComputePipeline* find_compute_pipeline(const Name& key) const;

		PipelineLibrary& release_render_resources() override;
	};
}// namespace Trinex
