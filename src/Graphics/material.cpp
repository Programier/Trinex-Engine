#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
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

	implement_engine_class(MaterialInterface, 0)
	{
		auto self = static_class_instance();

#define m_parameters m_child_objects
		auto params = trinex_refl_prop(self, This, m_parameters, Refl::Property::IsNotSerializable | Refl::Property::IsReadOnly);
#undef m_parameters

		Refl::Object::instance_cast<Refl::ObjectProperty>(params->element_property())->is_composite(true);
		params->tooltip("Array of parammeters of this material");
	}

	implement_engine_class(Material, Refl::Class::IsAsset)
	{
		auto* self = static_class_instance();
		trinex_refl_prop(self, This, compile_definitions);

		trinex_refl_prop(self, This, pipeline, Refl::Property::IsNotSerializable)
				->is_composite(true)
				.tooltip("Pipeline settings for this material");
	}

	implement_engine_class(MaterialInstance, Refl::Class::IsAsset)
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
		if (auto param = find_parameter(name))
		{
			param->owner(nullptr);
		}

		return *this;
	}

	MaterialInterface& MaterialInterface::clear_parameters()
	{
		auto params = std::move(m_child_objects);

		for (auto& param : params)
		{
			param->owner(nullptr);
		}

		return *this;
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
		pipeline = Object::new_instance<Pipeline>("Pipeline");
		pipeline->flags(Object::IsAvailableForGC, false);
		pipeline->owner(this);
	}

	bool Material::register_child(Object* child)
	{
		if (child == pipeline)
			return true;
		return ObjectTreeNode::register_child(child);
	}

	bool Material::unregister_child(Object* child)
	{
		if (child == pipeline)
			return true;
		return ObjectTreeNode::unregister_child(child);
	}

	Material& Material::preload()
	{
		pipeline->preload();
		return *this;
	}

	Material& Material::postload()
	{
		pipeline->postload();
		return *this;
	}

	bool Material::apply(SceneComponent* component, RenderPass* render_pass)
	{
		return apply(this, component, render_pass);
	}

	bool Material::apply(MaterialInterface* head, SceneComponent* component, RenderPass* render_pass)
	{
		trinex_check(is_in_render_thread(), "Material::apply method must be called in render thread!");

		// TODO: We need to find pipeline here using current render pass as id?
		pipeline->rhi_bind();

		for (auto& [name, info] : pipeline->parameters)
		{
			Parameter* parameter = head->find_parameter(name);

			if (parameter)
				parameter->apply(component, pipeline, render_pass, &info);
		}

		if (pipeline->global_parameters.has_parameters() && render_pass)
		{
			render_pass->scene_renderer()->bind_global_parameters(pipeline->global_parameters.bind_index());
		}
		return true;
	}

	class Material* Material::material()
	{
		return this;
	}

	bool Material::compile(ShaderCompiler::Compiler* compiler)
	{
		bool need_delete_compiler = compiler == nullptr;

		if (need_delete_compiler)
		{
			compiler = ShaderCompiler::Compiler::static_create_compiler();

			if (!compiler)
			{
				error_log("Material", "Failed to create material compiler!");

				if (need_delete_compiler)
				{
					delete compiler;
				}
			}
		}

		String slang_source;
		bool status = shader_source(slang_source);

		if (status == true)
		{
			ShaderCompiler::ShaderSource source;
			auto status = compiler->compile(this, slang_source, source);

			if (status)
			{
				status = submit_compiled_source(source);
			}
		}
		else
		{
			error_log("Material", "Failed to get shader source");
		}

		if (need_delete_compiler)
		{
			delete compiler;
		}

		return status;
	}

	Material& Material::apply_changes()
	{
		return postload();
	}

	bool Material::submit_compiled_source(const ShaderCompiler::ShaderSource& source)
	{
		bool status = pipeline->submit_compiled_source(source);
		if (!status)
			return status;


		TreeSet<Name> names_to_remove;

		for (auto& entry : parameters())
		{
			names_to_remove.insert(entry->name());
		}

		auto create_material_parameter = [&](Name name, Refl::Class* type) -> Parameter* {
			names_to_remove.erase(name);
			Parameter* material_parameter = find_parameter(name);

			if (material_parameter && material_parameter->class_instance() != type)
			{
				remove_parameter(name);
				material_parameter = nullptr;
			}

			if (!material_parameter)
			{
				if (!(material_parameter = Object::instance_cast<Parameter>(type->create_object(name, this))))
				{
					error_log("Material", "Failed to create material parameter '%s'", name.c_str());
					return nullptr;
				}
			}

			return material_parameter;
		};

		for (auto& [name, parameter] : pipeline->parameters)
		{
			trinex_always_check(parameter.type != nullptr, "Parameter type is nullptr");
			trinex_always_check(parameter.type->is_a<Parameter>(),
			                    "Material parameter type must be Engine::MaterialParameters::Parameter");
			create_material_parameter(name, parameter.type);
		}

		for (auto& name : names_to_remove)
		{
			remove_parameter(name);
		}

		apply_changes();
		status = true;


		return status;
	}

	bool Material::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;

		archive.serialize(compile_definitions);
		return pipeline->serialize(archive);
	}

	Material::~Material()
	{
		delete pipeline;
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
