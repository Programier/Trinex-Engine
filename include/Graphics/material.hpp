#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/object_tree_node.hpp>
#include <Core/object.hpp>
#include <Graphics/material_parameter.hpp>
#include <RHI/pipeline.hpp>

namespace Engine
{
	namespace MaterialParameters
	{
		class Parameter;
	}

	class Pipeline;
	class GraphicsPipeline;
	class MaterialBindings;
	struct RendererContext;
	class RenderPass;
	class ShaderCompiler;

	class ENGINE_EXPORT MaterialInterface : public ObjectTreeNode<Object, MaterialParameters::Parameter>
	{
		trinex_declare_class(MaterialInterface, Object);

	public:
		using Parameter = MaterialParameters::Parameter;

	protected:
		Refl::Class* object_tree_child_class() const override;
		bool unregister_child(Object* child) override;

	public:
		Parameter* find_parameter(const Name& name) const;
		const Vector<Parameter*>& parameters() const;

		template<typename T>
		T* find_parameter(const Name& name) const
		{
			return instance_cast<T>(find_parameter(name));
		}

		virtual MaterialInterface* parent() const;
		virtual class Material* material();
		virtual bool apply(const RendererContext& ctx, const MaterialBindings* bindings = nullptr);

		bool serialize(Archive& archive) override;
	};

	class ENGINE_EXPORT Material : public MaterialInterface
	{
		trinex_declare_class(Material, MaterialInterface);

	protected:
		Map<RenderPass*, GraphicsPipeline*> m_pipelines;

		bool register_child(Object* child) override;
		bool unregister_child(Object* child) override;
		bool apply_internal(MaterialInterface* head, const RendererContext& ctx, const MaterialBindings* bindings = nullptr);

	private:
		bool register_pipeline_parameters(GraphicsPipeline* pipeline);
		Material& remove_unreferenced_parameters();
		Material& remove_all_pipelines();
		bool compile_pass(ShaderCompiler* compiler, RenderPass* pass, const String& source);

	public:
		MaterialDomain domain;

		RHIDepthTest depth_test;
		RHIStencilTest stencil_test;
		RHIColorBlending color_blending;

		Material();
		GraphicsPipeline* pipeline(RenderPass* pass) const;
		bool add_pipeline(RenderPass* pass, GraphicsPipeline* pipeline);
		GraphicsPipeline* remove_pipeline(RenderPass* pass);
		Material& postload() override;
		Material& setup_pipeline(GraphicsPipeline* pipeline);

		class Material* material() override;
		bool apply(const RendererContext& ctx, const MaterialBindings* bindings = nullptr) override;
		bool serialize(Archive& archive) override;

		virtual bool compile(ShaderCompiler* compiler = nullptr, RenderPass* pass = nullptr);
		virtual bool shader_source(String& out_source) = 0;

		~Material();
		friend class MaterialInstance;
	};

	class ENGINE_EXPORT MaterialInstance : public MaterialInterface
	{
		trinex_declare_class(MaterialInstance, MaterialInterface);

	public:
		MaterialInterface* parent_material = nullptr;

		class Material* material() override;
		MaterialInterface* parent() const override;
		bool apply(const RendererContext& ctx, const MaterialBindings* bindings = nullptr) override;
		bool serialize(Archive& archive) override;
	};
}// namespace Engine
