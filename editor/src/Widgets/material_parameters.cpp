#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <Widgets/property_renderer.hpp>
#include <algorithm>

namespace Engine::MaterialParameters
{
	class ParametersTree
	{
		struct ParameterInfo {
			String m_name;
			Parameter* m_parameter = nullptr;
			size_t m_index         = Constants::index_none;

			bool is_valid(Material::ChildsArray& array)
			{
				if (array.size() <= m_index)
					return false;
				return array[m_index] == m_parameter;
			}

			bool render(Material::ChildsArray& array, Refl::Property* prop, PropertyRenderer* renderer, bool is_read_only)
			{
				Refl::Class* class_instance = m_parameter->class_instance();

				renderer->create_row();
				renderer->next_prop_name(m_name);

				if (class_instance->properties().size() == 1)
				{
					auto child_prop = class_instance->properties()[0];
					return renderer->render_property(m_parameter, child_prop, is_read_only | child_prop->is_read_only());
				}
				else
				{
					return renderer->render_property(&m_parameter, prop, is_read_only, false);
				}
			}
		};

		struct Result {
			bool changed = false;
			bool dirty   = false;

			Result& operator=(const Result& result)
			{
				changed = changed || result.changed;
				dirty   = dirty || result.dirty;
				return *this;
			}
		};

		Vector<ParametersTree> m_childs;
		Vector<ParameterInfo> m_parameters;
		String m_name;

		Result render_internal(Material::ChildsArray& array, Refl::Property* prop, PropertyRenderer* renderer, bool is_read_only)
		{
			Result result;

			for (auto& node : m_childs)
			{
				renderer->create_row();
				ImGui::Indent();

				if (renderer->collapsing_header(&node, "%s", node.m_name.c_str()))
				{
					result = node.render_internal(array, prop, renderer, is_read_only);
				}

				ImGui::Unindent();
			}

			for (auto& parameter : m_parameters)
			{
				bool parameter_valid = parameter.is_valid(array);
				result.dirty         = result.dirty || !parameter_valid;

				if (parameter_valid)
				{
					bool status    = parameter.render(array, prop, renderer, is_read_only);
					result.changed = result.changed || status;
				}
			}

			return result;
		}

		void add_parameter(Material::ChildsArray* array, size_t index, StringView property_name)
		{
			Parameter* parameter = array->at(index);

			if (parameter->class_instance()->properties().empty())
				return;

			ParametersTree* node = this;

			while (!property_name.empty())
			{
				StringView name = Strings::parse_token(property_name, ".", &property_name);

				if (property_name.empty())
				{
					ParameterInfo* info = std::lower_bound(
							node->m_parameters.begin(), node->m_parameters.end(), name,
							[](const ParameterInfo& param, const StringView& name) { return param.m_name < name; });

					if (info && info->m_name == name)
						error_log("MaterialProperties", "Parameter with name '%s' already exist!", info->m_name.c_str());
					else
					{
						info              = node->m_parameters.emplace(info);
						info->m_index     = index;
						info->m_name      = name;
						info->m_parameter = array->at(index);
					}
				}
				else
				{
					auto next_node = std::lower_bound(
							node->m_childs.begin(), node->m_childs.end(), name,
							[](const ParametersTree& node, const StringView& name) { return node.m_name < name; });

					if (next_node == nullptr || next_node->m_name != name)
					{
						node         = node->m_childs.emplace(next_node);
						node->m_name = name;
					}
					else
					{
						node = next_node;
					}
				}
			}
		}

	public:
		void build(Material::ChildsArray* array, Refl::Property* prop)
		{
			for (size_t i = 0, count = array->size(); i < count; ++i)
			{
				add_parameter(array, i, array->at(i)->name());
			}
		}

		bool render(Material::ChildsArray* array, Refl::ArrayProperty* prop, PropertyRenderer* renderer, bool is_read_only)
		{
			Result result = render_internal(*array, prop->element_property(), renderer, is_read_only);

			if (result.dirty)
			{
				m_childs.clear();
				m_parameters.clear();
				build(array, prop);
			}

			return result.changed;
		}
	};

	static bool render_material_parameters(Material* material, Refl::ArrayProperty* prop, PropertyRenderer* renderer,
										   bool is_read_only)
	{
		Material::ChildsArray* array = prop->address_as<Material::ChildsArray>(material);

		if (renderer->collapsing_header(prop))
		{
			Any& data            = renderer->userdata.get(array);
			ParametersTree* tree = nullptr;

			if (data.has_value())
			{
				tree = &data.cast<ParametersTree&>();
			}
			else
			{
				data = ParametersTree();
				tree = &data.cast<ParametersTree&>();
				tree->build(array, prop);
			}

			ImGui::Indent();
			auto res = tree->render(array, prop, renderer, is_read_only);
			ImGui::Unindent();
			return res;
		}

		return false;
	}

	static void on_init()
	{
		ScriptNamespaceScopedChanger changer("Editor::PropertyRenderers");

		auto material_params = ScriptEngine::register_function("bool render_material_properties(Engine::Object@ material, "
															   "Engine::Refl::Property@, Engine::PropertyRenderer@, bool)",
															   render_material_parameters);

		auto prop = Refl::Object::instance_cast<Refl::ArrayProperty>(
				Refl::Property::static_require("Engine::MaterialInterface::m_parameters"));
		prop->renderer(material_params);
	}

	static InitializeController init(on_init);
}// namespace Engine::MaterialParameters
