#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/object_tree_node.hpp>
#include <Core/object.hpp>
#include <Core/structures.hpp>
#include <Graphics/material_parameter.hpp>

namespace Engine
{
	namespace MaterialParameters
	{
		class Parameter;
	}

	class Logger;
	class SceneComponent;
	class Pipeline;
	class GraphicsPipeline;
	class RenderPass;
	class GraphicsPipelineDescription;

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
		virtual bool apply(SceneComponent* component = nullptr, RenderPass* render_pass = nullptr);

		bool serialize(Archive& archive) override;
	};

	class ENGINE_EXPORT Material : public MaterialInterface
	{
		trinex_declare_class(Material, MaterialInterface);

	protected:
		bool register_child(Object* child) override;
		bool unregister_child(Object* child) override;

		Map<Refl::RenderPassInfo*, Pipeline*> m_pipelines;
		GraphicsPipelineDescription* m_graphics_options = nullptr;

	private:
		bool register_pipeline_parameters(Pipeline* pipeline);

	public:
		MaterialDomain domain;
		MaterialOptions options;

		Vector<ShaderDefinition> compile_definitions;

		Material();
		Pipeline* pipeline(Refl::RenderPassInfo* pass) const;
		bool add_pipeline(Refl::RenderPassInfo* pass, Pipeline* pipeline);
		Pipeline* remove_pipeline(Refl::RenderPassInfo* pass);
		Material& remove_all_pipelines();
		Material& postload() override;
		Material& setup_pipeline(GraphicsPipeline* pipeline);

		class Material* material() override;
		bool apply(SceneComponent* component = nullptr, RenderPass* render_pass = nullptr) override;
		bool apply(MaterialInterface* head, SceneComponent* component = nullptr, RenderPass* render_pass = nullptr);
		bool serialize(Archive& archive) override;

		virtual Material& post_compile(Refl::RenderPassInfo* pass, Pipeline* pipeline);
		virtual bool shader_source(String& out_source) = 0;

		inline GraphicsPipelineDescription* graphics_description() const { return m_graphics_options; }

		~Material();
	};

	class ENGINE_EXPORT MaterialInstance : public MaterialInterface
	{
		trinex_declare_class(MaterialInstance, MaterialInterface);

	public:
		MaterialInterface* parent_material = nullptr;

		class Material* material() override;
		MaterialInterface* parent() const override;
		bool apply(SceneComponent* component = nullptr, RenderPass* render_pass = nullptr) override;
		bool serialize(Archive& archive) override;
	};
}// namespace Engine
