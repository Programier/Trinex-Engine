#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
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
		auto self   = static_class_instance();
		auto params = trinex_refl_prop(self, This, m_parameters, Refl::Property::IsNotSerializable | Refl::Property::IsReadOnly);

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

	static Vector<Parameter*>::iterator lower_bound(Vector<Parameter*>& params, const Name& name)
	{
		return std::lower_bound(params.begin(), params.end(), name,
		                        [](Parameter* param, const Name& name) -> bool { return param->name() < name; });
	}

	static Vector<Parameter*>::iterator search(Vector<Parameter*>& params, const Name& name)
	{
		auto it  = lower_bound(params, name);
		auto end = params.end();

		if (it == end)
			return end;

		if ((*it)->name() == name)
			return it;
		return end;
	}

	bool MaterialInterface::register_child_internal(Object* child, const Name& name)
	{
		auto param = Object::instance_cast<Parameter>(child);

		if (param == nullptr)
			return Super::register_child(child);

		if (find_parameter(name) != nullptr)
		{
			error_log("MaterialInterface", "Failed to register new parameter. Parameter with name '%s' already exist!",
			          name.c_str());
			return false;
		}

		m_parameters.insert(lower_bound(m_parameters, name), param);
		return true;
	}

	bool MaterialInterface::register_child(Object* object)
	{
		return register_child_internal(object, object->name());
	}

	bool MaterialInterface::unregister_child(Object* child)
	{
		if (!Object::instance_cast<Parameter>(child))
			return Super::unregister_child(child);

		m_parameters.erase(search(m_parameters, child->name()));
		return true;
	}

	bool MaterialInterface::rename_child_object(Object* object, StringView new_name)
	{
		return unregister_child(object) && register_child_internal(object, new_name);
	}

	Object* MaterialInterface::find_child_object(StringView name, bool recursive) const
	{
		if (recursive)
		{
			StringView current_name = Strings::parse_name_identifier(name, &name);

			if (Object* object = find_child_object(current_name, false))
			{
				if (!name.empty())
				{
					object = object->find_child_object(name, true);
				}
				return object;
			}

			return nullptr;
		}
		else
		{
			auto it = search(const_cast<Vector<Parameter*>&>(m_parameters), name);

			if (it == m_parameters.end())
				return nullptr;
			return *it;
		}
	}

	Parameter* MaterialInterface::find_parameter(const Name& name) const
	{
		auto params = const_cast<Vector<Parameter*>&>(m_parameters);
		auto it     = search(params, name);
		if (it == params.end())
			return nullptr;
		return *it;
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
		auto params = std::move(m_parameters);

		for (auto& param : params)
		{
			param->owner(nullptr);
		}

		return *this;
	}

	const Vector<Parameter*>& MaterialInterface::parameters() const
	{
		return m_parameters;
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

		size_t size = m_parameters.size();
		archive & size;

		if (archive.is_reading())
		{
			clear_parameters();

			while (size > 0)
			{
				--size;
				String name;
				archive & name;

				if (auto param = Object::load_object(name, archive.reader(), SerializationFlags::SkipObjectSearch))
				{
					param->owner(this);
				}
			}
		}
		else
		{
			for (auto param : m_parameters)
			{
				String name = param->name().to_string();
				archive & name;
				param->save(archive.writer());
			}
		}

		return true;
	}

	MaterialInterface::~MaterialInterface()
	{
		clear_parameters();
	}

	Material::Material()
	{
		pipeline = Object::new_instance<Pipeline>("Pipeline");
		pipeline->flags(Object::IsAvailableForGC, false);
		pipeline->owner(this);
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
				parameter->apply(component, pipeline, &info);
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

		archive & compile_definitions;
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

	MaterialInstance::~MaterialInstance()
	{
		auto params = std::move(m_parameters);

		for (auto& param : params)
		{
			param->owner(nullptr);
		}
	}
}// namespace Engine
