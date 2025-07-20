#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_bindings.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture_2D.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	using Parameter = MaterialInterface::Parameter;

	template<typename T>
	class MaterialParametersExt : public T
	{
	public:
		using T::T;

		Refl::Property* element_property() const override
		{
			constexpr Parameter* T::* null_prop = nullptr;
			const auto flags                    = Refl::Property::InlineSingleField;
			using Element                       = Refl::NativeProperty<null_prop>;

			static Refl::Property* prop = Refl::Object::new_instance<Element>(nullptr, StringView("Element"), flags);
			return prop;
		}

		const String& index_name(const void* context, size_t index) const override
		{
			const Refl::ArrayProperty* prop = this;
			const Parameter* parameter      = *prop->at_as<const Parameter*>(context, index);
			return parameter->name().to_string();
		}
	};

	struct MaterialBindingsVisitor {
	private:
		const RHIShaderParameterInfo* m_parameter;

		template<RHIShaderParameterType type, typename T>
		bool bind_scalar(const T& value) const
		{
			if (m_parameter->type.type_index() == type.type_index())
			{
				rhi->update_scalar_parameter(&value, m_parameter);
				return true;
			}
			return false;
		}

	public:
		inline MaterialBindingsVisitor(const RHIShaderParameterInfo* info) : m_parameter(info) {}

		bool operator()(bool value) const { return bind_scalar<RHIShaderParameterType::Bool>(value); }
		bool operator()(Vector2b value) const { return bind_scalar<RHIShaderParameterType::Bool2>(value); }
		bool operator()(Vector3b value) const { return bind_scalar<RHIShaderParameterType::Bool3>(value); }
		bool operator()(Vector4b value) const { return bind_scalar<RHIShaderParameterType::Bool4>(value); }

		bool operator()(int_t value) const { return bind_scalar<RHIShaderParameterType::Int>(value); }
		bool operator()(Vector2i value) const { return bind_scalar<RHIShaderParameterType::Int2>(value); }
		bool operator()(const Vector3i& value) const { return bind_scalar<RHIShaderParameterType::Int3>(value); }
		bool operator()(const Vector4i& value) const { return bind_scalar<RHIShaderParameterType::Int4>(value); }

		bool operator()(uint_t value) const { return bind_scalar<RHIShaderParameterType::UInt>(value); }
		bool operator()(Vector2u value) const { return bind_scalar<RHIShaderParameterType::UInt2>(value); }
		bool operator()(const Vector3u& value) const { return bind_scalar<RHIShaderParameterType::UInt3>(value); }
		bool operator()(const Vector4u& value) const { return bind_scalar<RHIShaderParameterType::UInt4>(value); }

		bool operator()(float value) const { return bind_scalar<RHIShaderParameterType::Float>(value); }
		bool operator()(Vector2f value) const { return bind_scalar<RHIShaderParameterType::Float2>(value); }
		bool operator()(const Vector3f& value) const { return bind_scalar<RHIShaderParameterType::Float3>(value); }
		bool operator()(const Vector4f& value) const { return bind_scalar<RHIShaderParameterType::Float4>(value); }

		bool operator()(const MaterialBindings::MemoryBlock& value) const { return false; }
		bool operator()(const MaterialBindings::CombinedSamplerImage& value) const { return false; }

		bool operator()(RHI_ShaderResourceView* value) const
		{
			static constexpr RHIShaderParameterType mask = RHIShaderParameterType::META_Texture |
			                                               RHIShaderParameterType::META_Buffer |
			                                               RHIShaderParameterType::META_TexelBuffer;
			if (m_parameter->type & mask)
			{
				rhi->bind_srv(value, m_parameter->binding);
				return true;
			}
			return false;
		}

		bool operator()(RHI_Sampler* value) const
		{
			static constexpr RHIShaderParameterType dst_type = RHIShaderParameterType::Sampler;

			if (m_parameter->type.type_index() == dst_type.type_index())
			{
				rhi->bind_sampler(value, m_parameter->binding);
				return true;
			}
			return false;
		}
	};


	trinex_implement_engine_class(MaterialInterface, 0)
	{
#define m_parameters m_child_objects
		auto params = trinex_refl_prop_ext(MaterialParametersExt, m_parameters,
		                                   Refl::Property::IsTransient | Refl::Property::IsReadOnly);
#undef m_parameters

		Refl::Object::instance_cast<Refl::ObjectProperty>(params->element_property())->is_composite(true);
		params->tooltip("Array of parammeters of this material");
	}

	trinex_implement_engine_class(Material, Refl::Class::IsAsset)
	{
		trinex_refl_prop(domain, Refl::Property::IsTransient);
		trinex_refl_prop(depth_test, Refl::Property::IsTransient);
		trinex_refl_prop(stencil_test, Refl::Property::IsTransient);
		trinex_refl_prop(color_blending, Refl::Property::IsTransient);
	}

	trinex_implement_engine_class(MaterialInstance, Refl::Class::IsAsset)
	{
		trinex_refl_prop(parent_material)->display_name("Parent Material").tooltip("Parent Material of this instance");
	}

	Refl::Class* MaterialInterface::object_tree_child_class() const
	{
		return MaterialParameters::Parameter::static_reflection();
	}

	bool MaterialInterface::unregister_child(Object* child)
	{
		bool result = ObjectTreeNode::unregister_child(child);
		return result || child->is_instance_of<MaterialParameters::Parameter>();
	}

	Parameter* MaterialInterface::find_parameter(const Name& name) const
	{
		return instance_cast<Parameter>(find_child_object(name));
	}

	const Vector<Parameter*>& MaterialInterface::parameters() const
	{
		return m_child_objects;
	}

	MaterialInterface* MaterialInterface::parent() const
	{
		return nullptr;
	}

	class Material* MaterialInterface::material()
	{
		return nullptr;
	}

	bool MaterialInterface::apply(const RendererContext& ctx, const MaterialBindings* bindings)
	{
		return false;
	}

	bool MaterialInterface::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;

		size_t size = m_child_objects.size();
		archive.serialize(size);

		if (archive.is_reading())
		{
			while (size > 0)
			{
				--size;
				String name;
				archive.serialize(name);

				if (auto param = Object::load_object(name, archive.reader(), SerializationFlags::SkipObjectSearch))
				{
					param->owner(this);
				}
			}
		}
		else
		{
			for (auto param : m_child_objects)
			{
				String name = param->name().to_string();
				archive.serialize(name);
				param->save(archive.writer());
			}
		}

		return true;
	}

	Material::Material() : domain(MaterialDomain::Surface) {}

	GraphicsPipeline* Material::pipeline(RenderPass* pass) const
	{
		auto it = m_pipelines.find(pass);
		if (it == m_pipelines.end())
			return nullptr;
		return it->second;
	}

	bool Material::register_pipeline_parameters(GraphicsPipeline* pipeline)
	{
		for (const RHIShaderParameterInfo& info : pipeline->parameters())
		{
			if (info.type & RHIShaderParameterType::META_ExcludeMaterialParameter)
				continue;

			if (Parameter* param = find_parameter(info.name))
			{
				if (param->type().type_index() == info.type.type_index())
				{
					++param->m_pipeline_refs;
					continue;
				}
				else if (param->m_pipeline_refs == 0)
				{
					param->owner(nullptr);
				}
				else
				{
					error_log("Material", "Ambiguous parameter type with name '%s'", info.name.c_str());
					return false;
				}
			}

			auto class_instance = Parameter::static_find_class(info.type);

			if (class_instance == nullptr)
			{
				error_log("Material", "Failed to find material parameter class");
				return false;
			}

			if (auto parameter = Object::instance_cast<Parameter>(class_instance->create_object(info.name, this)))
			{
				parameter->m_pipeline_refs = 1;
			}
			else
			{
				error_log("Material", "Failed to create material parameter '%s'", info.name.c_str());
				return false;
			}
		}
		return true;
	}

	bool Material::add_pipeline(RenderPass* pass, GraphicsPipeline* pipeline)
	{
		if (pass == nullptr)
		{
			error_log("Material", "Cannot bind pipeline to undefined pass!");
			return false;
		}

		render_thread()->wait();

		auto current_pipeline = Material::pipeline(pass);

		if (current_pipeline == pipeline)
			return false;

		if (pipeline == nullptr)
		{
			if (current_pipeline)
				remove_pipeline(pass);
			return true;
		}
		else if (current_pipeline)
		{
			remove_pipeline(pass);
		}

		Name name = pass ? pass->name() : "Default";

		if (!pipeline->rename(name, this))
			return false;

		if (!register_pipeline_parameters(pipeline))
			return false;

		m_pipelines[pass] = pipeline;
		setup_pipeline(pipeline);
		return true;
	}

	GraphicsPipeline* Material::remove_pipeline(RenderPass* pass)
	{
		auto it = m_pipelines.find(pass);
		if (it == m_pipelines.end())
			return nullptr;

		render_thread()->wait();
		GraphicsPipeline* pipeline = it->second;
		m_pipelines.erase(it);
		pipeline->owner(nullptr);

		for (const RHIShaderParameterInfo& info : pipeline->parameters())
		{
			if (Parameter* param = find_parameter(info.name))
			{
				--param->m_pipeline_refs;

				if (param->m_pipeline_refs == 0)
				{
					param->owner(nullptr);
				}
			}
		}

		return pipeline;
	}

	bool Material::register_child(Object* child)
	{
		if (child && child->is_instance_of<Pipeline>())
			return true;

		return ObjectTreeNode::register_child(child);
	}

	bool Material::unregister_child(Object* child)
	{
		if (child && child->is_instance_of<Pipeline>())
			return true;

		return ObjectTreeNode::unregister_child(child);
	}

	Material& Material::postload()
	{
		for (auto& pipeline : m_pipelines)
		{
			pipeline.second->postload();
		}

		// Checking that all registered passes supported by this material are compiled
		if (ShaderCompiler* compiler = ShaderCompiler::instance())
		{
			uint_t count         = 0;
			String material_name = full_name();

			for (auto pass = RenderPass::static_first_pass(); pass; pass = pass->next_pass())
			{
				if (pass->is_material_compatible(this) && pipeline(pass) == nullptr)
				{
					warn_log("Material", "Material '%s' should support pass '%s', however no pipeline was found. Recompiling...",
					         material_name.c_str(), pass->name().c_str());

					if (compile(compiler, pass))
					{
						++count;
					}
				}
			}

			if (count > 0)
			{
				info_log("Material", "'%d' pipelines have been compiled. Saving the material...", count);
				remove_unreferenced_parameters();
				save();
			}
		}

		return *this;
	}

	bool Material::apply(const RendererContext& ctx, const MaterialBindings* bindings)
	{
		return apply_internal(this, ctx, bindings);
	}

	bool Material::apply_internal(MaterialInterface* head, const RendererContext& ctx, const MaterialBindings* bindings)
	{
		trinex_check(is_in_render_thread(), "Material::apply method must be called in render thread!");

		auto pipeline_object = pipeline(ctx.render_pass);

		if (pipeline_object == nullptr)
			return false;

		pipeline_object->rhi_bind();

		if (bindings)
		{
			for (const RHIShaderParameterInfo& info : pipeline_object->parameters())
			{
				if (auto binding = bindings->find(info.name))
				{
					MaterialBindingsVisitor visitor(&info);

					if (std::visit(visitor, *binding))
						continue;
				}

				Parameter* parameter = head->find_parameter(info.name);

				if (parameter)
					parameter->apply(ctx, &info);
			}
		}
		else
		{
			for (const RHIShaderParameterInfo& info : pipeline_object->parameters())
			{
				Parameter* parameter = head->find_parameter(info.name);

				if (parameter)
					parameter->apply(ctx, &info);
			}
		}
		return true;
	}

	class Material* Material::material()
	{
		return this;
	}

	bool Material::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;

		archive.serialize(domain, depth_test, stencil_test, color_blending);

		size_t pipelines_count = m_pipelines.size();
		archive.serialize(pipelines_count);

		if (archive.is_saving())
		{
			for (auto& [pass, pipeline] : m_pipelines)
			{
				Name name = pass->name();
				archive.serialize(name);
				pipeline->serialize(archive, this);
			}
		}
		else
		{
			while (pipelines_count > 0)
			{
				Name name;
				archive.serialize(name);

				auto pass     = RenderPass::static_find(name);
				auto pipeline = new_instance<GraphicsPipeline>();

				add_pipeline(pass, pipeline);
				pipeline->serialize(archive, this);
				register_pipeline_parameters(pipeline);
				--pipelines_count;
			}
		}

		return archive;
	}

	Material& Material::setup_pipeline(GraphicsPipeline* pipeline)
	{
		pipeline->depth_test     = depth_test;
		pipeline->stencil_test   = stencil_test;
		pipeline->color_blending = color_blending;
		return *this;
	}

	Material& Material::remove_unreferenced_parameters()
	{
		for (ptrdiff_t i = static_cast<ptrdiff_t>(m_child_objects.size()) - 1; i >= 0; --i)
		{
			Parameter* parameter = m_child_objects[i];

			if (parameter->m_pipeline_refs == 0)
			{
				parameter->owner(nullptr);
			}
		}
		return *this;
	}

	Material& Material::remove_all_pipelines()
	{
		render_thread()->wait();
		for (auto& [pass, pipeline] : m_pipelines)
		{
			pipeline->owner(nullptr);

			for (const RHIShaderParameterInfo& info : pipeline->parameters())
			{
				if (Parameter* param = find_parameter(info.name))
				{
					trinex_check(param->m_pipeline_refs > 0, "Parameter is referenced by pipeline, but reference count == 0");
					--param->m_pipeline_refs;
				}
			}
		}

		m_pipelines.clear();
		return *this;
	}

	bool Material::compile_pass(ShaderCompiler* compiler, RenderPass* pass, const String& source)
	{
		Pointer<GraphicsPipeline> pipeline = remove_pipeline(pass);
		const bool new_pipeline            = pipeline == nullptr;

		if (new_pipeline)
		{
			pipeline = new_instance<GraphicsPipeline>(pass->name());
		}
		else
		{
			pipeline->clear();
		}

		ShaderCompiler::StackEnvironment env;
		ShaderCompilationResult result;

		env.add_source(source.c_str());
		pass->modify_shader_compilation_env(&env);
		pipeline->modify_compilation_env(&env);

		if (compiler->compile(&env, result))
		{
			result.initialize_pipeline(pipeline);
			pipeline->init_render_resources();
			add_pipeline(pass, pipeline);
			return true;
		}

		return false;
	}

	bool Material::compile(ShaderCompiler* compiler, RenderPass* pass)
	{
		if (compiler == nullptr)
		{
			compiler = ShaderCompiler::instance();

			if (compiler == nullptr)
			{
				error_log("Material", "Failed to find material compiler");
				return false;
			}
		}

		String source;
		if (!shader_source(source))
		{
			error_log("Material", "Failed to get material source");
			return false;
		}

		if (pass == nullptr)
		{
			remove_all_pipelines();

			bool success = true;

			for (auto pass = RenderPass::static_first_pass(); pass && success; pass = pass->next_pass())
			{
				if (pass->is_material_compatible(this))
				{
					success = compile_pass(compiler, pass, source);
				}
			}

			remove_unreferenced_parameters();
			return success;
		}
		else
		{
			bool result = compile_pass(compiler, pass, source);
			remove_unreferenced_parameters();
			return result;
		}
	}

	bool Material::shader_source(String& out_source)
	{
		return false;
	}

	Material::~Material()
	{
		for (auto& pipeline : m_pipelines)
		{
			pipeline.second->owner(nullptr);
		}
	}

	class Material* MaterialInstance::material()
	{
		if (parent_material)
		{
			return parent_material->material();
		}
		return nullptr;
	}

	MaterialInterface* MaterialInstance::parent() const
	{
		return parent_material;
	}

	bool MaterialInstance::apply(const RendererContext& ctx, const MaterialBindings* bindings)
	{
		Material* mat = material();

		if (!mat)
		{
			return false;
		}

		return mat->apply_internal(this, ctx, bindings);
	}

	bool MaterialInstance::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		return true;
	}
}// namespace Engine
