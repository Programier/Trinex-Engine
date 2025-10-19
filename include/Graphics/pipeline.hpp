#pragma once
#include <Core/filesystem/path.hpp>
#include <Graphics/render_resource.hpp>
#include <RHI/resource_ptr.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	class Shader;
	class RenderPass;
	class Logger;
	class ShaderCompilationEnvironment;
	class RHIPipeline;
	class ShaderCompilationResult;

	class ENGINE_EXPORT Pipeline : public RenderResource
	{
		trinex_declare_class(Pipeline, RenderResource);

	protected:
		static StringView pipeline_name_of(StringView name);
		static Object* package_of(StringView name);
		Shader* create_new_shader();

	protected:
		RHIResourcePtr<RHIPipeline> m_pipeline;

	private:
		Vector<RHIShaderParameterInfo> m_parameters;

	public:
		bool add_parameter(const RHIShaderParameterInfo& parameter, bool replace = false);
		bool remove_parameter(const Name& name);
		const RHIShaderParameterInfo* find_parameter(const Name& name) const;
		Pipeline& parameters(const Vector<RHIShaderParameterInfo>& parameters);
		inline const Vector<RHIShaderParameterInfo>& parameters() const { return m_parameters; }

		class Material* material() const;
		Pipeline& release_render_resources() override;
		virtual bool serialize(Archive& ar) final override;
		virtual bool serialize(Archive& ar, Material* material);
		virtual Pipeline& clear();
		virtual Pipeline& modify_compilation_env(ShaderCompilationEnvironment* env);

		inline RHIPipeline* rhi_pipeline() const { return m_pipeline; }
	};

	class ENGINE_EXPORT GraphicsPipeline : public Pipeline
	{
		trinex_declare_class(GraphicsPipeline, Pipeline);

	public:
		Vector<RHIVertexAttribute> vertex_attributes;
		RHIDepthTest depth_test;
		RHIStencilTest stencil_test;
		RHIColorBlending color_blending;

	private:
		Shader* m_vertex_shader               = nullptr;
		Shader* m_tessellation_control_shader = nullptr;
		Shader* m_tessellation_shader         = nullptr;
		Shader* m_geometry_shader             = nullptr;
		Shader* m_fragment_shader             = nullptr;

	public:
		~GraphicsPipeline();
		GraphicsPipeline& init_render_resources() override;
		GraphicsPipeline& postload() override;

		Shader* vertex_shader() const;
		Shader* fragment_shader() const;
		Shader* tessellation_control_shader() const;
		Shader* tessellation_shader() const;
		Shader* geometry_shader() const;

		Shader* vertex_shader(bool create = false);
		Shader* fragment_shader(bool create = false);
		Shader* tessellation_control_shader(bool create = false);
		Shader* tessellation_shader(bool create = false);
		Shader* geometry_shader(bool create = false);

		GraphicsPipeline& remove_vertex_shader();
		GraphicsPipeline& remove_fragment_shader();
		GraphicsPipeline& remove_tessellation_control_shader();
		GraphicsPipeline& remove_tessellation_shader();
		GraphicsPipeline& remove_geometry_shader();

		bool serialize(class Archive& archive, Material* material = nullptr) override;
	};

	class ENGINE_EXPORT ComputePipeline : public Pipeline
	{
		trinex_declare_class(ComputePipeline, Pipeline);

		Shader* m_shader = nullptr;

	public:
		ComputePipeline();
		~ComputePipeline();
		ComputePipeline& init_render_resources() override;
		inline Shader* compute_shader() const { return m_shader; }
	};

	class ENGINE_EXPORT GlobalGraphicsPipeline : public GraphicsPipeline
	{
	protected:
		GlobalGraphicsPipeline& load_pipeline();

	public:
		GlobalGraphicsPipeline(StringView name = "");

		bool serialize(Archive& ar, Material* material) override;
		virtual Path shader_path() const = 0;
		virtual void initialize()        = 0;
	};

	class ENGINE_EXPORT GlobalComputePipeline : public ComputePipeline
	{
	protected:
		GlobalComputePipeline& load_pipeline();

	public:
		GlobalComputePipeline(StringView name = "");

		bool serialize(Archive& ar, Material* material) override;
		virtual Path shader_path() const = 0;
		virtual void initialize()        = 0;
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
	Path shader_path() const override;                                                                                           \
	void initialize() override;                                                                                                  \
	~class_name();                                                                                                               \
	friend class Engine::Object;                                                                                                 \
                                                                                                                                 \
private:

#define trinex_implement_pipeline(class_name, path)                                                                              \
	static Engine::byte TRINEX_CONCAT(trinex_global_pipeline_, __LINE__) =                                                       \
	        static_cast<Engine::byte>(Engine::InitializeController([]() { class_name::create(); }, #class_name).id());           \
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
	Path class_name::shader_path() const                                                                                         \
	{                                                                                                                            \
		return path;                                                                                                             \
	}                                                                                                                            \
	void class_name::initialize()
}// namespace Engine
