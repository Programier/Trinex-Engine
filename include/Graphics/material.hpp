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
	struct RendererContext;
	class RenderPass;

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
		MaterialInterface& remove_parameter(const Name& name);
		MaterialInterface& clear_parameters();
		uint16_t remove_unreferenced_parameters();
		const Vector<Parameter*>& parameters() const;

		template<typename T>
		T* find_parameter(const Name& name) const
		{
			return instance_cast<T>(find_parameter(name));
		}

		virtual MaterialInterface* parent() const;
		virtual class Material* material();
		virtual bool apply(const RendererContext& ctx);

		bool serialize(Archive& archive) override;
	};

	class ENGINE_EXPORT Material : public MaterialInterface
	{
		trinex_declare_class(Material, MaterialInterface);

	protected:
		Map<RenderPass*, GraphicsPipeline*> m_pipelines;

		bool register_child(Object* child) override;
		bool unregister_child(Object* child) override;
		bool apply_internal(MaterialInterface* head, const RendererContext& ctx);

	private:
		bool register_pipeline_parameters(GraphicsPipeline* pipeline);

	public:
		MaterialDomain domain;

		RHIDepthTest depth_test;
		RHIStencilTest stencil_test;
		RHIColorBlending color_blending;

		Material();
		GraphicsPipeline* pipeline(RenderPass* pass) const;
		bool add_pipeline(RenderPass* pass, GraphicsPipeline* pipeline);
		GraphicsPipeline* remove_pipeline(RenderPass* pass);
		Material& remove_all_pipelines();
		Material& postload() override;
		Material& setup_pipeline(GraphicsPipeline* pipeline);

		class Material* material() override;
		bool apply(const RendererContext& ctx) override;
		bool serialize(Archive& archive) override;

		virtual Material& post_compile(RenderPass* pass, GraphicsPipeline* pipeline);
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
		bool apply(const RendererContext& ctx) override;
		bool serialize(Archive& archive) override;
	};
}// namespace Engine
