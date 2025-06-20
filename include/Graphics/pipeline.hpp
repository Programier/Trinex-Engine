#pragma once
#include <Core/filesystem/path.hpp>
#include <Graphics/render_resource.hpp>
#include <RHI/pipeline.hpp>
#include <RHI/resource_ptr.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	class VertexShader;
	class TessellationControlShader;
	class TessellationShader;
	class GeometryShader;
	class FragmentShader;
	class ComputeShader;
	class RenderPass;
	class Logger;
	class Shader;
	class ShaderCompilationEnvironment;
	class RHI_Pipeline;

	class ENGINE_EXPORT Pipeline : public RenderResource
	{
		trinex_declare_class(Pipeline, RenderResource);

	protected:
		static StringView pipeline_name_of(StringView name);
		static Object* package_of(StringView name);

		template<typename Type>
		Type* create_new_shader(const char* name, Type*& out)
		{
			if (out)
				return out;

			out = Object::new_instance<Type>(name);
			out->flags(Object::IsAvailableForGC, false);
			out->owner(this);
			return out;
		}

	protected:
		RHIResourcePtr<RHI_Pipeline> m_pipeline;

	private:
		Vector<RHIShaderParameterInfo> m_parameters;

	public:
		enum Type
		{
			Graphics,
			Compute,
		};

		static Pipeline* static_create_pipeline(Type type);

		bool add_parameter(const RHIShaderParameterInfo& parameter, bool replace = false);
		bool remove_parameter(const Name& name);
		const RHIShaderParameterInfo* find_parameter(const Name& name) const;
		Pipeline& parameters(const Vector<RHIShaderParameterInfo>& parameters);
		inline const Vector<RHIShaderParameterInfo>& parameters() const { return m_parameters; }

		class Material* material() const;
		const Pipeline& rhi_bind() const;
		Pipeline& release_render_resources() override;
		virtual bool serialize(Archive& ar) final override;
		virtual bool serialize(Archive& ar, Material* material);
		virtual Pipeline& clear();
		virtual Pipeline& modify_compilation_env(ShaderCompilationEnvironment* env);
		virtual Pipeline& post_compile(RenderPass* pass);

		inline RHI_Pipeline* rhi_pipeline() const { return m_pipeline; }

		virtual Shader* shader(ShaderType type) const                                = 0;
		virtual Shader* shader(ShaderType type, bool create = false)                 = 0;
		virtual ShaderType shader_types() const                                      = 0;
		virtual Pipeline& allocate_shaders(ShaderType flags = ShaderType::Undefined) = 0;
		virtual Pipeline& remove_shaders(ShaderType flags = ShaderType::Undefined)   = 0;
		virtual Type type() const                                                    = 0;
	};

	class ENGINE_EXPORT GraphicsPipeline : public Pipeline
	{
		trinex_declare_class(GraphicsPipeline, Pipeline);

	public:
		RHIDepthTest depth_test;
		RHIStencilTest stencil_test;
		RHIColorBlending color_blending;

	private:
		VertexShader* m_vertex_shader                            = nullptr;
		TessellationControlShader* m_tessellation_control_shader = nullptr;
		TessellationShader* m_tessellation_shader                = nullptr;
		GeometryShader* m_geometry_shader                        = nullptr;
		FragmentShader* m_fragment_shader                        = nullptr;

	public:
		~GraphicsPipeline();
		GraphicsPipeline& init_render_resources() override;
		GraphicsPipeline& postload() override;

		Shader* shader(ShaderType type) const override;
		Shader* shader(ShaderType type, bool create = false) override;
		VertexShader* vertex_shader() const;
		FragmentShader* fragment_shader() const;
		TessellationControlShader* tessellation_control_shader() const;
		TessellationShader* tessellation_shader() const;
		GeometryShader* geometry_shader() const;

		VertexShader* vertex_shader(bool create = false);
		FragmentShader* fragment_shader(bool create = false);
		TessellationControlShader* tessellation_control_shader(bool create = false);
		TessellationShader* tessellation_shader(bool create = false);
		GeometryShader* geometry_shader(bool create = false);

		GraphicsPipeline& remove_vertex_shader();
		GraphicsPipeline& remove_fragment_shader();
		GraphicsPipeline& remove_tessellation_control_shader();
		GraphicsPipeline& remove_tessellation_shader();
		GraphicsPipeline& remove_geometry_shader();

		ShaderType shader_types() const override;
		GraphicsPipeline& allocate_shaders(ShaderType flags = ShaderType::Undefined) override;
		GraphicsPipeline& remove_shaders(ShaderType flags = ShaderType::Undefined) override;
		Type type() const override;
		bool serialize(class Archive& archive, Material* material = nullptr) override;
	};

	class ENGINE_EXPORT ComputePipeline : public Pipeline
	{
		trinex_declare_class(ComputePipeline, Pipeline);

		ComputeShader* m_shader = nullptr;

	public:
		~ComputePipeline();
		ComputePipeline& init_render_resources() override;
		Shader* shader(ShaderType type) const override;
		Shader* shader(ShaderType type, bool create = false) override;
		ShaderType shader_types() const override;
		ComputePipeline& allocate_shaders(ShaderType flags = ShaderType::Undefined) override;
		ComputePipeline& remove_shaders(ShaderType flags = ShaderType::Undefined) override;
		Type type() const override;

		inline ComputeShader* compute_shader() const { return m_shader; }
	};

	class ENGINE_EXPORT GlobalGraphicsPipeline : public GraphicsPipeline
	{
	protected:
		GlobalGraphicsPipeline& load_pipeline();

	public:
		GlobalGraphicsPipeline(StringView name = "", ShaderType types = {});

		bool serialize(Archive& ar, Material* material) override;
		virtual Path shader_path() const = 0;
		virtual void initialize()        = 0;
	};

	class ENGINE_EXPORT GlobalComputePipeline : public ComputePipeline
	{
	protected:
		GlobalComputePipeline& load_pipeline();

	public:
		GlobalComputePipeline(StringView name = "", ShaderType types = {});

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

#define trinex_implement_pipeline(class_name, path, shaders)                                                                     \
	static Engine::byte TRINEX_CONCAT(trinex_global_pipeline_, __LINE__) =                                                       \
	        static_cast<Engine::byte>(Engine::InitializeController([]() { class_name::create(); }, #class_name).id());           \
                                                                                                                                 \
	class_name* class_name::s_instance = nullptr;                                                                                \
	class_name::class_name() : Super(#class_name, static_cast<Engine::ShaderType::Enum>(shaders))                                \
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
