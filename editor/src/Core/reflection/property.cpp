#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/actor_component.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <Widgets/properties_window.hpp>
#include <angelscript.h>

namespace Engine::Refl
{
	static inline bool boolean_xor(bool a, bool b)
	{
		return a ? !b : b;
	}

	static void property_renderer(Refl::Property* prop, asIScriptFunction** func_ptr, int_t type_id)
	{
		// Check function desc
		asIScriptFunction* func = *func_ptr;

		trinex_always_check(func->GetReturnTypeId() == asTYPEID_BOOL, "Return type of the renderer function must be bool!");
		trinex_always_check(
				func->GetParamCount() == 4,
				"The rendering function must have 4 arguments\n"
				"\t\t\t1: Handle or reference to the object whose property is being edited\n"
				"\t\t\t2: Handle to Engine::Refl::Property\n"
				"\t\t\t3: Handle to Engine::PropertyRenderer\n"
				"\t\t\t4: Boolean variable, which will contain the value true if the property is rendered in read only mode");

		asDWORD flags = 0;
		func->GetParam(0, &type_id, &flags);

		const bool is_handle =
				ScriptEngine::is_handle_type(type_id) || ScriptEngine::type_info_by_id(type_id).is_implicit_handle();
		const bool is_reference = (flags & asTM_INREF) == asTM_INREF;

		trinex_always_check(boolean_xor(is_handle, is_reference),
							"The 1-th argument of renderer func must be handle or reference to the object whose "
							"property is being edited");

		static int_t second_arg = ScriptEngine::type_id_by_decl("Engine::Refl::Property@");
		static int_t third_arg  = ScriptEngine::type_id_by_decl("Engine::PropertyRenderer@");

		func->GetParam(1, &type_id, &flags);
		trinex_always_check(type_id == second_arg && (flags & asTM_INREF) == 0,
							"The 2-th argument of renderer func must be handle to Engine::Refl::Property");

		func->GetParam(2, &type_id, &flags);
		trinex_always_check(type_id == third_arg && (flags & asTM_INREF) == 0,
							"The 2-th argument of renderer func must be handle to Engine::PropertyRenderer");

		func->GetParam(3, &type_id, &flags);
		trinex_always_check(type_id == asTYPEID_BOOL && (flags & asTM_INREF) == 0,
							"The 2-th argument of renderer func must be handle to Engine::PropertyRenderer");

		// Function is valid, we can apply it
		prop->renderer(func);
	}

	static bool render_object_array_element(Engine::Object** object, Refl::Property* prop, ImGuiObjectProperties* renderer,
											bool is_read_only)
	{
		renderer->next_prop_name((*object)->name().to_string());
		return renderer->render_property(object, prop, is_read_only, false);
	}

	static bool render_material_parameter(Engine::Object** address, Refl::Property* prop, ImGuiObjectProperties* renderer,
										  bool is_read_only)
	{
		Engine::Object* object = *address;
		auto class_instance    = object->class_instance();
		auto properties_count  = class_instance->properties().size();

		if (properties_count == 0)
		{
			renderer->mark_property_skipped();
			return false;
		}

		renderer->next_prop_name((*address)->name().to_string());

		if (properties_count == 1)
		{
			auto child_prop = class_instance->properties()[0];
			return renderer->render_property(object, child_prop, is_read_only | child_prop->is_read_only());
		}
		else
		{
			return renderer->render_property(address, prop, is_read_only, false);
		}
	}

	static void on_refl_init()
	{
		ReflectionInitializeController().require("Engine::Refl::Property");
		auto r = ScriptClassRegistrar::existing_class("Engine::Refl::Property");
		r.method("void renderer(?&)", property_renderer);
	}

	static void on_init()
	{
		ScriptNamespaceScopedChanger changer("Editor::PropertyRenderers");
		auto object_ell = ScriptEngine::register_function("bool render_object_element(Engine::Object@, "
														  "Engine::Refl::Property@, Engine::PropertyRenderer@, bool)",
														  render_object_array_element);

		auto material_prop_ell = ScriptEngine::register_function("bool render_material_property_element(Engine::Object@, "
																 "Engine::Refl::Property@, Engine::PropertyRenderer@, bool)",
																 render_material_parameter);

		ArrayProperty* prop = Object::instance_cast<ArrayProperty>(Property::static_require("Engine::Actor::m_owned_components"));
		prop->element_property()->renderer(object_ell);

		prop = Object::instance_cast<ArrayProperty>(Property::static_require("Engine::MaterialInterface::m_parameters"));
		prop->element_property()->renderer(material_prop_ell);
	}

	static ReflectionInitializeController refl_init(on_refl_init, "Engine::Refl::Property", {"Engine::Actor"});
	static InitializeController init(on_init);
}// namespace Engine::Refl
