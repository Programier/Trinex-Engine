#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/object_tree_node.hpp>
#include <Core/deprecated/object.hpp>
#include <Graphics/enums.hpp>
#include <Graphics/material_parameter.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	namespace MaterialParameters
	{
		class Parameter;
	}

	class Pipeline;
	class GraphicsPipeline;
	class MaterialBindings;
	struct PrimitiveRenderingContext;
	class RenderPass;
	class ShaderCompiler;

	class trinex_api MaterialInterface : public ObjectTreeNode<DeprecatedObject, MaterialParameters::Parameter>
	{
		trinex_class(MaterialInterface, DeprecatedObject);

	public:
		using Parameter = MaterialParameters::Parameter;

	protected:
		bool unregister_child(DeprecatedObject* child) override;

		Parameter* create_parameter(const Name& name, RHIShaderParameterType type);

	public:
		Parameter* find_parameter(const Name& name) const;
		const Array<Parameter*>& parameters() const;

		template<typename T>
		T* find_parameter(const Name& name) const
		{
			return instance_cast<T>(find_parameter(name));
		}

		virtual MaterialInterface* parent() const;
		virtual class Material* material();
		virtual bool apply(const PrimitiveRenderingContext* ctx);

		bool serialize(Archive& archive) override;
	};

	class trinex_api Material : public MaterialInterface
	{
		trinex_class(Material, MaterialInterface);

	protected:
		Map<RenderPass*, GraphicsPipeline*> m_pipelines;

		DeprecatedObject* register_child(DeprecatedObject* child, u32& index) override;
		bool unregister_child(DeprecatedObject* child) override;
		bool apply_internal(MaterialInterface* head, const PrimitiveRenderingContext* ctx);

	private:
		bool register_pipeline_parameters(GraphicsPipeline* pipeline);
		Material& remove_unreferenced_parameters();
		Material& remove_all_pipelines();
		bool compile_pass(ShaderCompiler* compiler, RenderPass* pass, const String& source);

	public:
		trinex_property(IsTransient)
		MaterialDomain domain;

		trinex_property(IsTransient)
		MaterialDepthMode depth_mode;

		trinex_property(IsTransient)
		MaterialBlendMode blend_mode;

		Material();
		GraphicsPipeline* pipeline(RenderPass* pass) const;
		bool add_pipeline(RenderPass* pass, GraphicsPipeline* pipeline);
		GraphicsPipeline* remove_pipeline(RenderPass* pass);
		Material& postload() override;

		class Material* material() override;
		bool apply(const PrimitiveRenderingContext* ctx) override;
		bool serialize(Archive& archive) override;

		virtual bool compile(ShaderCompiler* compiler = nullptr, RenderPass* pass = nullptr);
		virtual bool shader_source(String& out_source) = 0;

		~Material();
		friend class MaterialInstance;
	};

	class trinex_api MaterialInstance : public MaterialInterface
	{
		trinex_class(MaterialInstance, MaterialInterface);

	private:
		MaterialInterface* m_parent = nullptr;

	public:
		MaterialInstance();
		Material* material() override;
		MaterialInstance& parent(MaterialInterface* material);
		MaterialInterface* parent() const override;
		bool apply(const PrimitiveRenderingContext* ctx) override;

		using MaterialInterface::create_parameter;

		template<typename T>
		T* create_parameter(const Name& name)
		{
			return static_cast<T*>(MaterialInterface::create_parameter(name, T::static_type()));
		}
	};
} // namespace Trinex
