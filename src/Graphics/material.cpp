#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
	using Parameter = MaterialInterface::Parameter;

	trinex_implement_engine_class(MaterialInterface, 0)
	{
		auto self = static_class_instance();

#define m_parameters m_child_objects
		auto params = trinex_refl_prop(self, This, m_parameters, Refl::Property::IsNotSerializable | Refl::Property::IsReadOnly);
#undef m_parameters

		Refl::Object::instance_cast<Refl::ObjectProperty>(params->element_property())->is_composite(true);
		params->tooltip("Array of parammeters of this material");
	}

	trinex_implement_engine_class(Material, Refl::Class::IsAsset)
	{
		auto* self = static_class_instance();
		trinex_refl_prop(self, This, compile_definitions);
		trinex_refl_prop(self, This, m_graphics_options, Refl::Property::IsNotSerializable)->is_composite(true);

		trinex_refl_prop(self, This, domain);
		trinex_refl_prop(self, This, options);
	}

	trinex_implement_engine_class(MaterialInstance, Refl::Class::IsAsset)
	{
		auto* self = MaterialInstance::static_class_instance();
		trinex_refl_prop(self, This, parent_material)
				->display_name("Parent Material")
				.tooltip("Parent Material of this instance");
	}

	Refl::Class* MaterialInterface::object_tree_child_class() const
	{
		return MaterialParameters::Parameter::static_class_instance();
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

	MaterialInterface& MaterialInterface::remove_parameter(const Name& name)
	{
		render_thread()->wait();
		if (auto param = find_parameter(name))
		{
			param->m_pipeline_refs = 0;
			param->owner(nullptr);
		}

		return *this;
	}

	MaterialInterface& MaterialInterface::clear_parameters()
	{
		render_thread()->wait();
		auto params = std::move(m_child_objects);

		for (auto& param : params)
		{
			param->owner(nullptr);
		}

		return *this;
	}

	uint16_t MaterialInterface::remove_unreferenced_parameters()
	{
		uint16_t count = 0;

		for (ptrdiff_t i = static_cast<ptrdiff_t>(m_child_objects.size()) - 1; i >= 0; --i)
		{
			Parameter* parameter = m_child_objects[i];

			if (parameter->m_pipeline_refs == 0)
			{
				++count;
				parameter->owner(nullptr);
			}
		}

		return count;
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

	bool MaterialInterface::apply(SceneComponent* component, RenderPass* render_pass)
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
			clear_parameters();

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

	Material::Material()
	{
		m_graphics_options = new_instance<GraphicsPipelineDescription>();
		m_graphics_options->add_reference();
	}

	Pipeline* Material::pipeline(Refl::RenderPassInfo* pass) const
	{
		if (options & MaterialOptions::DefaultPassOnly)
			pass = nullptr;

		auto it = m_pipelines.find(pass);
		if (it == m_pipelines.end())
			return nullptr;
		return it->second;
	}

	bool Material::register_pipeline_parameters(Pipeline* pipeline)
	{
		for (auto& [name, info] : pipeline->parameters)
		{
			if (Parameter* param = find_parameter(name))
			{
				if (param->type() == info.type)
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
					error_log("Material", "Ambiguous parameter type with name '%s'", name.c_str());
					return false;
				}
			}

			auto class_instance = Parameter::static_find_class(info.type);

			if (class_instance == nullptr)
			{
				error_log("Material", "Failed to find material parameter class");
				return false;
			}

			if (auto parameter = Object::instance_cast<Parameter>(class_instance->create_object(name, this)))
			{
				parameter->m_pipeline_refs = 1;
			}
			else
			{
				error_log("Material", "Failed to create material parameter '%s'", name.c_str());
				return false;
			}
		}
		return true;
	}

	bool Material::add_pipeline(Refl::RenderPassInfo* pass, Pipeline* pipeline)
	{
		render_thread()->wait();

		if (Material::pipeline(pass) == pipeline)
		{
			error_log("Material", "Cannot add pipeline, because current pipeline already exist in material!");
			return false;
		}

		if (pipeline == nullptr)
		{
			remove_pipeline(pass);
			return true;
		}

		Name name = pass ? pass->name() : "Default";

		if (!pipeline->rename(name, this))
			return false;

		if (!register_pipeline_parameters(pipeline))
			return false;

		m_pipelines[pass] = pipeline;

		if (auto graphics_pipeline = instance_cast<GraphicsPipeline>(pipeline))
		{
			setup_pipeline(graphics_pipeline);
		}
		return true;
	}

	Pipeline* Material::remove_pipeline(Refl::RenderPassInfo* pass)
	{
		auto it = m_pipelines.find(pass);
		if (it == m_pipelines.end())
			return nullptr;

		render_thread()->wait();
		Pipeline* pipeline = it->second;
		m_pipelines.erase(it);
		pipeline->owner(nullptr);

		for (auto& [name, info] : pipeline->parameters)
		{
			if (Parameter* param = find_parameter(name))
			{
				--param->m_pipeline_refs;
			}
		}

		return pipeline;
	}

	Material& Material::remove_all_pipelines()
	{
		render_thread()->wait();
		for (auto& [pass, pipeline] : m_pipelines)
		{
			pipeline->owner(nullptr);

			for (auto& [name, info] : pipeline->parameters)
			{
				if (Parameter* param = find_parameter(name))
				{
					trinex_check(param->m_pipeline_refs > 0, "Parameter is referenced by pipeline, but reference count == 0");
					--param->m_pipeline_refs;
				}
			}
		}

		m_pipelines.clear();
		return *this;
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
		// Checking that all registered passes supported by this material are compiled
		if (ShaderCompiler* compiler = ShaderCompiler::instance())
		{
			uint_t count         = 0;
			String material_name = full_name();

			for (auto pass = Refl::RenderPassInfo::first_pass(); pass; pass = pass->next_pass())
			{
				if (pass->is_material_compatible(this) && pipeline(pass) == nullptr)
				{
					warn_log("Material", "Material '%s' should support pass '%s', however no pipeline was found. Recompiling...",
							 material_name.c_str(), pass->name().c_str());

					if (compiler->compile_pass(this, pass))
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

		for (auto& pipeline : m_pipelines)
		{
			pipeline.second->postload();
		}

		return *this;
	}

	bool Material::apply(SceneComponent* component, RenderPass* render_pass)
	{
		return apply(this, component, render_pass);
	}

	bool Material::apply(MaterialInterface* head, SceneComponent* component, RenderPass* render_pass)
	{
		trinex_check(is_in_render_thread(), "Material::apply method must be called in render thread!");

		auto pipeline_object = pipeline(render_pass ? render_pass->info() : nullptr);

		if (pipeline_object == nullptr)
			return false;

		pipeline_object->rhi_bind();

		for (auto& [name, info] : pipeline_object->parameters)
		{
			Parameter* parameter = head->find_parameter(name);

			if (parameter)
				parameter->apply(component, render_pass, &info);
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

		archive.serialize(compile_definitions);

		bool with_graphics_options = m_graphics_options != nullptr;
		archive.serialize(with_graphics_options);

		if (with_graphics_options)
		{
			m_graphics_options->serialize(archive);
		}

		// Serialize pipelines

		size_t pipelines_count = m_pipelines.size();
		archive.serialize(pipelines_count);

		if (archive.is_saving())
		{
			for (auto& pipeline : m_pipelines)
			{
				Name name           = pipeline.first ? pipeline.first->name() : "Default";
				Pipeline::Type type = pipeline.second->type();

				archive.serialize(name);
				archive.serialize(type);

				pipeline.second->serialize(archive, this);
			}
		}
		else
		{
			while (pipelines_count > 0)
			{
				Name name;
				Pipeline::Type type;

				archive.serialize(name);
				archive.serialize(type);

				if (name == Name::none)
				{
					name = "Default";
				}

				auto pass     = Refl::RenderPassInfo::static_find_pass(name);
				auto pipeline = Pipeline::static_create_pipeline(type);

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
		if (m_graphics_options)
		{
			pipeline->depth_test     = m_graphics_options->depth_test;
			pipeline->stencil_test   = m_graphics_options->stencil_test;
			pipeline->input_assembly = m_graphics_options->input_assembly;
			pipeline->rasterizer     = m_graphics_options->rasterizer;
			pipeline->color_blending = m_graphics_options->color_blending;
		}
		return *this;
	}

	Material& Material::post_compile(Refl::RenderPassInfo* pass, Pipeline* pipeline)
	{
		return *this;
	}

	Material::~Material()
	{
		for (auto& pipeline : m_pipelines)
		{
			pipeline.second->owner(nullptr);
		}

		m_graphics_options->remove_reference();
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

	bool MaterialInstance::apply(SceneComponent* component, RenderPass* render_pass)
	{
		Material* mat = material();

		if (!mat)
		{
			return false;
		}

		return mat->apply(this, component, render_pass);
	}

	bool MaterialInstance::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		return true;
	}
}// namespace Engine
