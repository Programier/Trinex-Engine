#include <Core/engine_loading_controllers.hpp>
#include <Core/group.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/script_class.hpp>
#include <Core/reflection/script_enum.hpp>
#include <Core/reflection/script_property.hpp>
#include <Core/reflection/script_struct.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>
#include <regex>

namespace Engine
{
	static Map<String, Script::PropertyReflectionParser> m_custom_parsers;

	enum class MetaType
	{
		Unknown,
		Flags,
		Assignment,
		FunctionCall
	};

	static bool is_child_of_object(const ScriptTypeInfo& info)
	{
		auto p_info      = info.info();
		auto object_info = Engine::Object::static_reflection()->script_type_info.info();

		while (p_info && p_info != object_info)
		{
			p_info = p_info->GetBaseType();
		}

		return p_info == object_info;
	}

	static MetaType determine_metadata_type(const String& str)
	{
		std::regex flags_regex(R"(^[a-zA-Z_][a-zA-Z0-9_]*$)");
		std::regex assignment_regex(R"(^[a-zA-Z_][a-zA-Z0-9_]*\s*=\s*.+$)");
		std::regex function_call_regex(R"(^[a-zA-Z_][a-zA-Z0-9_]*\s*\(.*\)$)");

		if (std::regex_match(str, flags_regex))
		{
			return MetaType::Flags;
		}
		else if (std::regex_match(str, assignment_regex))
		{
			return MetaType::Assignment;
		}
		else if (std::regex_match(str, function_call_regex))
		{
			return MetaType::FunctionCall;
		}

		return MetaType::Unknown;
	}

	static void register_flags_meta(Refl::Property* prop, const String& meta)
	{
		struct Accessor : public Refl::Property {
			static void add_flag(Refl::Property* prop, BitMask mask)
			{
				constexpr BitMask Property::* address = &Accessor::m_flags;
				auto& flags                           = prop->*address;
				flags |= mask;
			}
		};

		static asITypeInfo* enum_type = ScriptEngine::engine()->GetTypeInfoByDecl("Engine::Refl::Property::Flag");

		asUINT count = enum_type->GetEnumValueCount();

		for (asUINT i = 0; i < count; i++)
		{
			int64_t value = 0;

			if (meta == enum_type->GetEnumValueByIndex(i, &value))
			{
				Accessor::add_flag(prop, value);
				return;
			}
		}

		throw EngineException(Strings::format("Failed to find property flag with name '{}'", meta));
	}

	static void register_expression_meta(Script* script, Refl::Object* self, const String& meta)
	{
		const Refl::ClassInfo* refl_info = self->refl_class_info();
		while (!refl_info->is_scriptable) refl_info = refl_info->parent;

		if (refl_info == nullptr)
			throw EngineException(Strings::format("Cannot find scriptable property type for prop '{}'", self->full_name()));

		String code = Strings::format("void __trinex_engine_execute_meta__(Engine::Refl::{}@ prop) {{ prop.{}; }}",
		                              refl_info->class_name.to_string(), meta);

		auto module    = script->module().as_module();
		const char* ns = module->GetDefaultNamespace();

		{
			String func_ns = self->owner()->scope_name();
			module->SetDefaultNamespace(func_ns.c_str());
		}

		String section              = Strings::format("{}: {}: MetaData", script->name(), self->full_name());
		asIScriptFunction* function = nullptr;
		if (module->CompileFunction(section.c_str(), code.c_str(), 0, 0, &function) < 0)
			throw EngineException(Strings::format("Failed to bind meta '{}'", meta));

		ScriptContext::prepare(function);
		ScriptContext::arg_address(0, self);
		ScriptContext::execute();
		ScriptContext::unprepare();

		function->Release();
		module->SetDefaultNamespace(ns);
	}

	static void register_metadata(Script* script, Refl::Property* prop, const TreeSet<String>& metadata)
	{
		for (auto& meta : metadata)
		{
			auto type = determine_metadata_type(meta);

			switch (type)
			{
				case MetaType::Flags: register_flags_meta(prop, meta); break;
				case MetaType::Assignment:
				case MetaType::FunctionCall: register_expression_meta(script, prop, meta);
				default: break;
			}
		}
	}

	static void register_metadata(Script* script, Refl::Struct* self, const TreeSet<String>& metadata)
	{
		for (auto& meta : metadata)
		{
			auto type = determine_metadata_type(meta);

			switch (type)
			{
				case MetaType::Assignment:
				case MetaType::FunctionCall: register_expression_meta(script, self, meta);
				default: break;
			}
		}
	}

	static Refl::Property* register_enum_property(Script* script, Refl::Struct* self, const String& prop_name,
	                                              ScriptTypeInfo info, size_t offset)
	{
		auto fullname             = Strings::concat_scoped_name(info.namespace_name(), info.name());
		Refl::Enum* enum_instance = Refl::Enum::static_find(fullname);

		if (enum_instance == nullptr)
			enum_instance = Refl::Object::new_instance<Refl::ScriptEnum>(fullname, script, info);

		return self->new_child<Refl::ScriptEnumProperty>(prop_name, offset, enum_instance);
	}

	static Refl::Property* register_primitive_property(Script* script, Refl::Struct* self, int_t type_id, uint_t prop_idx)
	{
		uint_t offset = self->script_type_info.property_offset(prop_idx);

		String name          = String(self->script_type_info.property_name(prop_idx));
		Refl::Property* prop = nullptr;

		if (ScriptEngine::is_bool(type_id))
		{
			prop = self->new_child<Refl::ScriptBooleanProperty>(name, offset);
		}
		else if (ScriptEngine::is_int8(type_id))
		{
			prop = self->new_child<Refl::ScriptIntegerProperty<int8_t>>(name, offset);
		}
		else if (ScriptEngine::is_int16(type_id))
		{
			prop = self->new_child<Refl::ScriptIntegerProperty<int16_t>>(name, offset);
		}
		else if (ScriptEngine::is_int32(type_id))
		{
			prop = self->new_child<Refl::ScriptIntegerProperty<int32_t>>(name, offset);
		}
		else if (ScriptEngine::is_int64(type_id))
		{
			prop = self->new_child<Refl::ScriptIntegerProperty<int64_t>>(name, offset);
		}
		else if (ScriptEngine::is_uint8(type_id))
		{
			prop = self->new_child<Refl::ScriptIntegerProperty<uint8_t>>(name, offset);
		}
		else if (ScriptEngine::is_uint16(type_id))
		{
			prop = self->new_child<Refl::ScriptIntegerProperty<uint16_t>>(name, offset);
		}
		else if (ScriptEngine::is_uint32(type_id))
		{
			prop = self->new_child<Refl::ScriptIntegerProperty<uint32_t>>(name, offset);
		}
		else if (ScriptEngine::is_uint64(type_id))
		{
			prop = self->new_child<Refl::ScriptIntegerProperty<uint64_t>>(name, offset);
		}
		else if (ScriptEngine::is_float(type_id))
		{
			prop = self->new_child<Refl::ScriptFloatProperty<float>>(name, offset);
		}
		else if (ScriptEngine::is_double(type_id))
		{
			prop = self->new_child<Refl::ScriptFloatProperty<double>>(name, offset);
		}

		ScriptTypeInfo info = ScriptEngine::type_info_by_id(type_id);
		if (!info.is_valid())
			return prop;

		if (info.is_enum())
		{
			return register_enum_property(script, self, name, info, offset);
		}

		return prop;
	}

	static Refl::Property* register_class_property(Script* script, Refl::Struct* self, ScriptTypeInfo info, uint_t prop_idx,
	                                               bool has_property_meta)
	{
		auto decl = Strings::concat_scoped_name(info.namespace_name(), info.name());
		{
			auto it = m_custom_parsers.find(decl);
			if (it != m_custom_parsers.end())
				return (*it).second(script, self, info, prop_idx);
		}

		if (!has_property_meta && !is_child_of_object(info))
			return nullptr;

		auto prop_struct = script->create_reflection(info);

		if (!prop_struct)
			return nullptr;

		uint_t offset = self->script_type_info.property_offset(prop_idx);

		String name          = String(self->script_type_info.property_name(prop_idx));
		Refl::Property* prop = nullptr;

		if (prop_struct->is_class())
		{
			Refl::Class* instance = Refl::Object::instance_cast<Refl::Class>(prop_struct);
			if (instance)
			{
				prop = self->new_child<Refl::ScriptObjectProperty>(name, offset, instance);
			}
		}
		else if (has_property_meta)
		{
			bool is_handle = ScriptEngine::is_handle_type(self->script_type_info.property_type_id(prop_idx));
			trinex_always_check(!is_handle, "Cannot register handle as struct property!");
			prop = self->new_child<Refl::ScriptStructProperty>(name, offset, prop_struct);
		}

		return prop;
	}

	static void register_reflection(Script* script, Refl::Struct* self)
	{
		auto& type      = self->script_type_info;
		auto prop_count = type.property_count();

		String type_name     = Strings::concat_scoped_name(type.namespace_name(), type.name());
		auto& class_metadata = script->metadata_for_class(type_name);

		register_metadata(script, self, class_metadata.class_metadata);

		for (uint_t i = 0; i < prop_count; ++i)
		{
			if (!type.is_property_native(i))
			{
				auto prop_type_id = type.property_type_id(i);
				auto& metadata    = class_metadata.metadata_for_property(i);

				Refl::Property* prop = nullptr;

				if (ScriptEngine::is_primitive_type(prop_type_id) && metadata.contains("property"))
				{
					prop = register_primitive_property(script, self, prop_type_id, i);
				}
				else if (ScriptEngine::is_object_type(prop_type_id, true))
				{
					ScriptTypeInfo info = ScriptEngine::type_info_by_id(prop_type_id);
					prop                = register_class_property(script, self, info, i, metadata.contains("property"));
				}

				if (prop)
				{
					register_metadata(script, prop, metadata);
				}
			}
		}
	}

	void Script::register_custom_reflection_parser(StringView datatype, PropertyReflectionParser parser)
	{
		m_custom_parsers[String(datatype)] = parser;
	}

	Refl::Struct* Script::create_reflection(const ScriptTypeInfo& info)
	{
		if (!info.is_valid())
			return nullptr;

		auto decl = Strings::concat_scoped_name(info.namespace_name(), info.name());

		if (auto instance = Refl::Struct::static_find(decl))
		{
			if (instance->script_type_info == info)
				return instance;

			throw EngineException(
			        Strings::format("Cannot register script class, because class with name '{}' already exist!", decl));
		}

		if (info.is_native())
			throw EngineException(Strings::format("Cannot create script reflection for native type '{}'", decl));

		auto base       = info.base_type();
		auto base_class = base.is_valid() ? create_reflection(base) : nullptr;

		if (base.is_valid() && !base_class)
			return nullptr;

		Refl::Struct* script_class = nullptr;

		if (base_class && base_class->is_class())
		{
			if (auto parent = Refl::Object::instance_cast<Refl::Class>(base_class))
			{
				script_class = Refl::Object::new_instance<Refl::ScriptClass>(decl, parent, this, info);
			}
		}
		else
		{
			script_class = Refl::Object::new_instance<Refl::ScriptStruct>(
			        decl, Refl::Object::instance_cast<Refl::ScriptStruct>(base_class), this, info);
		}

		if (script_class)
		{
			register_reflection(this, script_class);
		}
		return script_class;
	}


	static Refl::Property* string_property(Script* script, Refl::Struct* self, ScriptTypeInfo info, uint_t idx)
	{
		bool is_handle = ScriptEngine::is_handle_type(self->script_type_info.property_type_id(idx));
		trinex_always_check(!is_handle, "Cannot register handle as string property!");

		uint_t offset = self->script_type_info.property_offset(idx);
		String name   = String(self->script_type_info.property_name(idx));
		return self->new_child<Refl::ScriptStringProperty>(name, offset);
	}

	static void on_preinit()
	{
		Script::register_custom_reflection_parser("string", string_property);
	}

	static PreInitializeController preinit(on_preinit);
}// namespace Engine
