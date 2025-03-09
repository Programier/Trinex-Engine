#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/map.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/pointer.hpp>
#include <Core/render_resource.hpp>
#include <Core/structures.hpp>

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

	class GraphicsPipelineDescription : public Object
	{
		trinex_declare_class(GraphicsPipelineDescription, Object);

	public:
		struct DepthTestInfo {
			trinex_declare_struct(DepthTestInfo, void);

			CompareFunc func  = CompareFunc::Less;
			bool enable       = true;
			bool write_enable = true;
		} depth_test;

		struct StencilTestInfo {
			trinex_declare_struct(StencilTestInfo, void);

			bool enable          = false;
			StencilOp fail       = StencilOp::Decr;
			StencilOp depth_pass = StencilOp::Decr;
			StencilOp depth_fail = StencilOp::Decr;
			CompareFunc compare  = CompareFunc::Less;
			byte compare_mask    = 0;
			byte write_mask      = 0;
		} stencil_test;

		struct AssemblyInfo {
			trinex_declare_struct(AssemblyInfo, void);

			PrimitiveTopology primitive_topology = PrimitiveTopology::TriangleList;
		} input_assembly;

		struct RasterizerInfo {
			trinex_declare_struct(RasterizerInfo, void);

			PolygonMode polygon_mode = PolygonMode::Fill;
			CullMode cull_mode       = CullMode::None;
			FrontFace front_face     = FrontFace::ClockWise;
			float line_width         = 1.f;
		} rasterizer;

		struct ColorBlendingInfo {
			trinex_declare_struct(ColorBlendingInfo, void);

			bool enable               = false;
			BlendFunc src_color_func  = BlendFunc::SrcAlpha;
			BlendFunc dst_color_func  = BlendFunc::OneMinusSrcAlpha;
			BlendOp color_op          = BlendOp::Add;
			BlendFunc src_alpha_func  = BlendFunc::One;
			BlendFunc dst_alpha_func  = BlendFunc::Zero;
			BlendOp alpha_op          = BlendOp::Add;
			ColorComponent color_mask = static_cast<ColorComponent::Enum>(ColorComponent::R | ColorComponent::G |
																		  ColorComponent::B | ColorComponent::A);
		} color_blending;

		bool serialize(Archive& ar) override;
	};

	class ENGINE_EXPORT Pipeline : public RenderResource
	{
		trinex_declare_class(Pipeline, RenderResource);

	protected:
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

		RenderResourcePtr<RHI_Pipeline> m_pipeline;

	public:
		enum Type
		{
			Graphics,
			Compute,
		};

		TreeMap<Name, ShaderParameterInfo> parameters;

		static Pipeline* static_create_pipeline(Type type);

		class Material* material() const;
		const ShaderParameterInfo* find_param_info(const Name& name) const;
		const Pipeline& rhi_bind() const;
		Pipeline& release_render_resources() override;
		virtual bool serialize(Archive& ar) final override;
		virtual bool serialize(Archive& ar, Material* material);
		virtual Pipeline& clear();
		virtual bool shader_source(String& source);

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
		GraphicsPipelineDescription::DepthTestInfo depth_test;
		GraphicsPipelineDescription::StencilTestInfo stencil_test;
		GraphicsPipelineDescription::AssemblyInfo input_assembly;
		GraphicsPipelineDescription::RasterizerInfo rasterizer;
		GraphicsPipelineDescription::ColorBlendingInfo color_blending;

	private:
		VertexShader* m_vertex_shader                            = nullptr;
		TessellationControlShader* m_tessellation_control_shader = nullptr;
		TessellationShader* m_tessellation_shader                = nullptr;
		GeometryShader* m_geometry_shader                        = nullptr;
		FragmentShader* m_fragment_shader                        = nullptr;

	public:
		GraphicsPipeline();
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
		bool shader_source(String& source) override;
		bool serialize(class Archive& archive, Material* material = nullptr) override;
	};

	class ENGINE_EXPORT ComputePipeline : public Pipeline
	{
		trinex_declare_class(ComputePipeline, Pipeline);

		ComputeShader* m_shader = nullptr;

	public:
		Path shader_path;

		ComputePipeline& init_render_resources() override;
		Shader* shader(ShaderType type) const override;
		Shader* shader(ShaderType type, bool create = false) override;
		ShaderType shader_types() const override;
		ComputePipeline& allocate_shaders(ShaderType flags = ShaderType::Undefined) override;
		ComputePipeline& remove_shaders(ShaderType flags = ShaderType::Undefined) override;
		Type type() const override;
		bool shader_source(String& source) override;
	};
}// namespace Engine
