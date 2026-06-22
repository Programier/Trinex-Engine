#pragma once
#include <Core/etl/map.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/name.hpp>
#include <Core/pointer.hpp>
#include <Graphics/render_resource.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	class Object;
	class Pipeline;
	class GraphicsPipeline;
	class ComputePipeline;
	class ShaderCompiler;
	class ShaderCompilationEnvironment;
	class ShaderCompilationResult;
	class RHIPipeline;
	struct PipelineLibraryCache;

	class ENGINE_EXPORT PipelineLibrary : public RenderResource
	{
		trinex_class(PipelineLibrary, RenderResource);

	public:
		using Pipelines = TreeMap<Name, Pointer<Pipeline>>;

	protected:
		Path m_shader_path;
		Pipelines m_pipelines;

		bool compile_permutation(const ShaderCompilationResult& result);
		void clear_pipelines();

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

	class ENGINE_EXPORT GlobalPipelineLibrary : public PipelineLibrary
	{
		trinex_class(GlobalPipelineLibrary, PipelineLibrary);

	private:
		bool load_default_pipeline_cache();

	protected:
		static StringView pipeline_name_of(StringView name);
		static Object* package_of(StringView name);

		GlobalPipelineLibrary& load_pipeline();
		String permutation_cache_name(Name name) const override;

	public:
		GlobalPipelineLibrary(StringView name = "");

		Pipeline* pipeline(Name permutation = {}) const;
		GraphicsPipeline* graphics_pipeline(Name permutation = {}) const;
		ComputePipeline* compute_pipeline(Name permutation = {}) const;
		RHIPipeline* handle(Name permutation = {}) const;
		const RHIShaderParameterInfo* find_parameter(const Name& key, Name permutation = {}) const;

		virtual Path shader_source_path() const = 0;
		virtual void initialize()               = 0;
	};

#define trinex_declare_pipeline(class_name, base_class)                                                                          \
private:                                                                                                                         \
	static class_name* s_instance;                                                                                               \
                                                                                                                                 \
	class_name();                                                                                                                \
                                                                                                                                 \
public:                                                                                                                          \
	using Super = base_class;                                                                                                    \
	using This  = class_name;                                                                                                    \
                                                                                                                                 \
	static class_name* create();                                                                                                 \
	static inline class_name* instance()                                                                                         \
	{                                                                                                                            \
		return s_instance;                                                                                                       \
	}                                                                                                                            \
	Path shader_source_path() const override;                                                                                    \
	void initialize() override;                                                                                                  \
	~class_name();                                                                                                               \
	friend class Trinex::Object;                                                                                                 \
                                                                                                                                 \
private:

#define trinex_implement_pipeline(class_name, path)                                                                              \
	trinex_on_init({.name = #class_name})                                                                                        \
	{                                                                                                                            \
		class_name::create();                                                                                                    \
	}                                                                                                                            \
                                                                                                                                 \
	class_name* class_name::s_instance = nullptr;                                                                                \
	class_name::class_name() : Super(#class_name)                                                                                \
	{                                                                                                                            \
		s_instance = this;                                                                                                       \
	}                                                                                                                            \
	class_name::~class_name()                                                                                                    \
	{                                                                                                                            \
		s_instance = nullptr;                                                                                                    \
	}                                                                                                                            \
	class_name* class_name::create()                                                                                             \
	{                                                                                                                            \
		if (!s_instance)                                                                                                         \
		{                                                                                                                        \
			Object::new_instance<class_name>(pipeline_name_of(#class_name), package_of(#class_name));                            \
			s_instance->add_reference();                                                                                         \
			s_instance->load_pipeline();                                                                                         \
		}                                                                                                                        \
		return s_instance;                                                                                                       \
	}                                                                                                                            \
	Path class_name::shader_source_path() const                                                                                  \
	{                                                                                                                            \
		return path;                                                                                                             \
	}                                                                                                                            \
	void class_name::initialize()
}// namespace Trinex
