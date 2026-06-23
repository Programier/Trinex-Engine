#pragma once
#include <Core/etl/object_tree_node.hpp>
#include <Graphics/pipeline.hpp>

namespace Trinex
{
	class Pipeline;
	class GraphicsPipeline;
	class ComputePipeline;
	class ShaderCompiler;
	class ShaderCompilationEnvironment;
	class ShaderCompilationResult;
	class RHIPipeline;

	struct PipelineLibraryCache;
	struct RHIShaderParameterInfo;

	class ENGINE_EXPORT PipelineLibrary : public ObjectTreeNode<RenderResource, Pipeline>
	{
		trinex_class(PipelineLibrary, RenderResource);

	protected:
		Pipeline* create_pipeline_instance(u8 type, Name name);
		String permutation_cache_name(Name name) const;

	public:
		Pipeline* find_pipeline(const Name& key) const;
		GraphicsPipeline* find_graphics_pipeline(const Name& key) const;
		ComputePipeline* find_compute_pipeline(const Name& key) const;
	};

	class ENGINE_EXPORT GlobalPipelineLibrary : public PipelineLibrary
	{
		trinex_class(GlobalPipelineLibrary, PipelineLibrary);

	private:
		bool load_default_pipeline_cache();
		bool compile_permutation(const ShaderCompilationResult& result);
		bool compile(ShaderCompiler* compiler = nullptr);

	protected:
		static StringView pipeline_name_of(StringView name);
		static Object* package_of(StringView name);

		GlobalPipelineLibrary& load_pipeline();

		virtual GlobalPipelineLibrary& modify_compilation_env(ShaderCompilationEnvironment* env);

	public:
		GlobalPipelineLibrary(StringView name = "");

		Pipeline* pipeline(Name permutation = {}) const;
		GraphicsPipeline* graphics_pipeline(Name permutation = {}) const;
		ComputePipeline* compute_pipeline(Name permutation = {}) const;
		RHIPipeline* handle(Name permutation = {}) const;
		const RHIShaderParameterInfo* find_parameter(const Name& key, Name permutation = {}) const;

		virtual const char* source_path() const = 0;
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
	const char* source_path() const override;                                                                                    \
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
	const char* class_name::source_path() const                                                                                  \
	{                                                                                                                            \
		return path;                                                                                                             \
	}                                                                                                                            \
	void class_name::initialize()
}// namespace Trinex
