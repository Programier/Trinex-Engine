#pragma once
#include <Core/asset.hpp>
#include <Core/types/path.hpp>
#include <RHI/resource_ptr.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	class Shader;
	class RenderPass;
	class Logger;
	class ShaderCompilationEnvironment;
	class RHIPipeline;
	class ShaderCompilationResult;

	class ENGINE_EXPORT Pipeline : public Asset
	{
		trinex_class(Pipeline, Asset);

	protected:
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
		virtual bool serialize(Archive& ar) final override;
		virtual bool serialize(Archive& ar, Material* material);
		virtual Pipeline& clear();
		virtual Pipeline& modify_compilation_env(ShaderCompilationEnvironment* env);

		inline RHIPipeline* handle() const { return m_pipeline; }
	};

	class ENGINE_EXPORT GraphicsPipeline : public Pipeline
	{
		trinex_class(GraphicsPipeline, Pipeline);

	public:
		Vector<RHIInputAttribute> vertex_attributes;

	private:
		Shader* m_vertex_shader               = nullptr;
		Shader* m_tessellation_control_shader = nullptr;
		Shader* m_tessellation_shader         = nullptr;
		Shader* m_geometry_shader             = nullptr;
		Shader* m_fragment_shader             = nullptr;

	public:
		~GraphicsPipeline();
		GraphicsPipeline& rebuild() override;
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
		trinex_class(ComputePipeline, Pipeline);

		Shader* m_shader = nullptr;

	public:
		ComputePipeline();
		~ComputePipeline();
		ComputePipeline& rebuild() override;
		inline Shader* compute_shader() const { return m_shader; }
	};
}// namespace Trinex
