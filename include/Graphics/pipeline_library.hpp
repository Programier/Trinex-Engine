#pragma once
#include <Core/etl/map.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/pointer.hpp>
#include <Graphics/render_resource.hpp>
#include <Graphics/shader_compiler.hpp>

namespace Trinex
{
	class Pipeline;
	class GraphicsPipeline;
	class ComputePipeline;
	class ShaderCompiler;
	struct PipelineLibraryCache;

	class ENGINE_EXPORT PipelineLibrary : public RenderResource
	{
		trinex_class(PipelineLibrary, RenderResource);

	public:
		using Pipelines = TreeMap<String, Pointer<Pipeline>>;

	private:
		Path m_shader_path;
		Vector<ShaderPermutationDescriptor> m_permutations;
		Pipelines m_pipelines;

		bool compile_permutation(const ShaderCompilationResult& result);
		void clear_pipelines();

	protected:
		virtual PipelineLibrary& modify_compilation_env(ShaderCompilationEnvironment* env);
		virtual Pipeline* create_pipeline_instance(u8 type, const ShaderPermutationDescriptor& permutation);
		virtual PipelineLibrary& initialize_pipeline(Pipeline* pipeline, const ShaderPermutationDescriptor& permutation);
		virtual String permutation_cache_name(const ShaderPermutationDescriptor& permutation) const;

	public:
		PipelineLibrary();
		~PipelineLibrary();

		PipelineLibrary& shader_path(const Path& path);
		const Path& shader_path() const;
		const Vector<ShaderPermutationDescriptor>& permutations() const;
		const Pipelines& pipelines() const;

		// Permutations are discovered from the source module via:
		// [TrinexPipeline("VariantName")]
		// [TrinexSpecialize("Expression")]
		bool compile(ShaderCompiler* compiler = nullptr);
		Pipeline* find_pipeline(const ShaderPermutationKey& key) const;
		GraphicsPipeline* find_graphics_pipeline(const ShaderPermutationKey& key) const;
		ComputePipeline* find_compute_pipeline(const ShaderPermutationKey& key) const;

		PipelineLibrary& release_render_resources() override;
	};
}// namespace Trinex
