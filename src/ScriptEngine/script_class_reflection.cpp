#include <Core/class.hpp>
#include <Core/group.hpp>
#include <Core/property.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>
#include <regex>


namespace Engine
{
	static int_t vec2_type_id   = 0;
	static int_t vec3_type_id   = 0;
	static int_t vec4_type_id   = 0;
	static int_t name_type_id   = 0;
	static int_t string_type_id = 0;


	// virtual void* prop_address(void* object)									   = 0;
	// virtual const void* prop_address(const void* object) const					   = 0;
	// virtual PropertyValue property_value(const void* object) const				   = 0;

	// virtual bool property_value(void* object, const PropertyValue& property_value) = 0;
	// virtual size_t size() const													   = 0;
	// virtual size_t min_alignment() const										   = 0;
	// virtual PropertyType type() const											   = 0;

	struct ScriptPrimitivePropBase : public Property {
		uint_t m_offset;

		ScriptPrimitivePropBase(uint_t offset, const Name& name, const String& description = "", const Name& group = Name::none,
		                        BitMask flags = 0)
		    : Property(name, description, group, flags)
		{
			m_offset = offset;
		}

		void* prop_address(void* object) override
		{
			if (object == nullptr)
				return nullptr;
			return reinterpret_cast<byte*>(object) + m_offset;
		}

		const void* prop_address(const void* object) const override
		{
			if (object == nullptr)
				return nullptr;
			return reinterpret_cast<const byte*>(object) + m_offset;
		}
	};

	template<typename T>
	struct ScriptPrimitiveProp : public ScriptPrimitivePropBase {
		uint_t m_offset = 0;

		using ScriptPrimitivePropBase::ScriptPrimitivePropBase;

		size_t size() const override
		{
			return sizeof(T);
		}

		size_t min_alignment() const override
		{
			return alignof(DataType);
		}

		PropertyValue property_value(const void* object) const override
		{
			if (object)
			{
				return PropertyValue(*reinterpret_cast<const T*>(prop_address(object)));
			}
			return {};
		}

		bool property_value(void* object, const PropertyValue& property_value) override
		{
			if (!ScriptPrimitivePropBase::is_const() && object /* && property_value.type() == prop_type*/)
			{
				(*reinterpret_cast<T*>(prop_address(object))) = static_cast<T>(property_value.cast<T>());
				Property::on_prop_changed(object);
				return true;
			}
			return false;
		}

		PropertyType type() const override
		{
			return PropertyType::Undefined;
		}

		size_t type_id() const override
		{
			return Engine::type_info<T>::id();
		}
	};


	struct ScriptObjectPropBase : public Property {
		uint_t m_offset;

		ScriptObjectPropBase(uint_t offset, const Name& name, const String& description = "", const Name& group = Name::none,
		                     BitMask flags = 0)
		    : Property(name, description, group, flags)
		{
			m_offset = offset;
		}

		void* prop_address(void* object) override
		{
			if (object == nullptr)
				return nullptr;
			return *reinterpret_cast<void**>(reinterpret_cast<byte*>(object) + m_offset);
		}

		const void* prop_address(const void* object) const override
		{
			if (object == nullptr)
				return nullptr;
			return *reinterpret_cast<void* const*>(reinterpret_cast<const byte*>(object) + m_offset);
		}
	};

	template<typename T, PropertyType prop_type = PropertyType::Undefined>
	struct ScriptObjectProp : public ScriptObjectPropBase {
		uint_t m_offset = 0;

		using ScriptObjectPropBase::ScriptObjectPropBase;

		size_t size() const override
		{
			return sizeof(T);
		}

		size_t min_alignment() const override
		{
			return alignof(DataType);
		}

		PropertyValue property_value(const void* object) const override
		{
			if (object)
			{
				return PropertyValue(*reinterpret_cast<const T*>(prop_address(object)));
			}
			return {};
		}

		bool property_value(void* object, const PropertyValue& property_value) override
		{

			if (!ScriptObjectPropBase::is_const() && object && property_value.type() == prop_type)
			{
				(*reinterpret_cast<T*>(prop_address(object))) = static_cast<T>(property_value.cast<T>());
				Property::on_prop_changed(object);
				return true;
			}
			return false;
		}

		PropertyType type() const override
		{
			return prop_type;
		}
	};


	static String parse_name_argument(const String& input)
	{
		std::regex pattern("\\bname\\(\\s*\"([^\"]*)\"\\s*\\)");
		std::smatch match;
		if (std::regex_search(input, match, pattern))
		{
			return match[1].str();
		}
		return "";
	}

	static String parse_desc_argument(const String& input)
	{
		std::regex pattern("\\bdesc\\(\\s*\"([^\"]*)\"\\s*\\)");
		std::smatch match;
		if (std::regex_search(input, match, pattern))
		{
			return match[1].str();
		}
		return "";
	}

	static String parse_group_argument(const String& input)
	{
		std::regex pattern("\\bgroup\\(\\s*\"([^\"]*)\"\\s*\\)");
		std::smatch match;
		if (std::regex_search(input, match, pattern))
		{
			return match[1].str();
		}
		return "";
	}


	static void parse_metadata(const TreeSet<String>& metadata, String& name, String& desc, String& group, BitMask& flags)
	{
		for (auto& argument : metadata)
		{
			if (argument.starts_with("name"))
			{
				String result = parse_name_argument(argument);
				if (!result.empty())
				{
					name = result;
				}
			}
			else if (argument.starts_with("desc"))
			{
				String result = parse_desc_argument(argument);
				if (!result.empty())
				{
					desc = result;
				}
			}
			else if (argument.starts_with("group"))
			{
				String result = parse_group_argument(argument);
				if (!result.empty())
				{
					group = result;
				}
			}
			else if (argument == "hidden")
			{
				flags |= Property::Flag::IsHidden;
			}
		}
	}

	static void register_primitive_property(Class* self, const TreeSet<String>& metadata, int_t type_id, uint_t prop_idx)
	{
		if (!metadata.contains("property"))
			return;

		uint_t offset = self->script_type_info.property_offset(prop_idx);

		if (offset == 0)
			return;

		String name   = String(self->script_type_info.property_name(prop_idx));
		String desc   = "";
		String group  = "";
		BitMask flags = 0;

		parse_metadata(metadata, name, desc, group, flags);

		Property* prop = nullptr;

		if (ScriptEngine::is_bool(type_id))
		{
			prop = new ScriptPrimitiveProp<bool>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_int8(type_id))
		{
			prop = new ScriptPrimitiveProp<int8_t>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_int16(type_id))
		{
			prop = new ScriptPrimitiveProp<int16_t>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_int32(type_id))
		{
			prop = new ScriptPrimitiveProp<int32_t>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_int64(type_id))
		{
			prop = new ScriptPrimitiveProp<int64_t>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_uint8(type_id))
		{
			prop = new ScriptPrimitiveProp<uint8_t>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_uint16(type_id))
		{
			prop = new ScriptPrimitiveProp<uint16_t>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_uint32(type_id))
		{
			prop = new ScriptPrimitiveProp<uint32_t>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_uint64(type_id))
		{
			prop = new ScriptPrimitiveProp<uint64_t>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_float(type_id))
		{
			prop = new ScriptPrimitiveProp<float>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_double(type_id))
		{
			prop = new ScriptPrimitiveProp<double>(offset, name, desc, group, flags);
		}

		self->add_property(prop);
	}

	static void initialize_classes_type_id()
	{
		if (string_type_id != 0)
			return;

		vec2_type_id   = ScriptEngine::type_id_by_decl("Engine::Vector2D");
		vec3_type_id   = ScriptEngine::type_id_by_decl("Engine::Vector3D");
		vec4_type_id   = ScriptEngine::type_id_by_decl("Engine::Vector4D");
		name_type_id   = ScriptEngine::type_id_by_decl("Engine::Name");
		string_type_id = ScriptEngine::type_id_by_decl("string");
	}


	static void register_object_property(Class* self, const TreeSet<String>& metadata, int_t type_id, uint_t prop_idx)
	{
		Property* prop = nullptr;

		uint_t offset = self->script_type_info.property_offset(prop_idx);

		if (offset == 0)
			return;

		initialize_classes_type_id();

		String name   = String(self->script_type_info.property_name(prop_idx));
		String desc   = "";
		String group  = "";
		BitMask flags = 0;

		parse_metadata(metadata, name, desc, group, flags);

		if (type_id == vec2_type_id)
		{
			prop = new ScriptObjectProp<Vector2D>(offset, name, desc, group, flags);
		}
		else if (type_id == vec3_type_id)
		{
			if (metadata.contains("is_color"))
				prop = new ScriptPrimitiveProp<Vector3D>(offset, name, desc, group, flags);
			else
				prop = new ScriptPrimitiveProp<Vector3D>(offset, name, desc, group, flags);
		}
		else if (type_id == vec4_type_id)
		{
			if (metadata.contains("is_color"))
				prop = new ScriptPrimitiveProp<Vector4D>(offset, name, desc, group, flags);
			else
				prop = new ScriptPrimitiveProp<Vector4D>(offset, name, desc, group, flags);
		}
		else if (type_id == name_type_id)
		{
			prop = new ScriptObjectProp<Name>(offset, name, desc, group, flags);
		}
		else if (type_id == string_type_id)
		{
			prop = new ScriptObjectProp<String>(offset, name, desc, group, flags);
		}
		else
		{
			auto type = ScriptEngine::type_info_by_id(type_id);
		}

		self->add_property(prop);
	}

	static void register_class_metadata(Class* self, const TreeSet<String>& metadata)
	{
		for (auto& argument : metadata)
		{
			if (argument.starts_with("group"))
			{
				String result = parse_group_argument(argument);
				if (!result.empty())
				{
					self->group(Group::find(result, true));
				}
			}
		}
	}

	Script& Script::register_reflection(Class* self)
	{
		auto& type      = self->script_type_info;
		auto prop_count = type.property_count();

		String type_name     = Strings::concat_scoped_name(type.namespace_name(), type.name());
		auto& class_metadata = metadata_for_class(type_name);

		if (prop_count > 0)
		{
			for (uint_t i = 0; i < prop_count; ++i)
			{
				if (!type.is_property_native(i))
				{
					auto prop_type_id = type.property_type_id(i);
					auto& metadata    = class_metadata.metadata_for_property(i);

					if (ScriptEngine::is_primitive_type(prop_type_id))
					{
						register_primitive_property(self, metadata, prop_type_id, i);
						continue;
					}

					if (ScriptEngine::is_object_type(prop_type_id, true))
					{
						register_object_property(self, metadata, prop_type_id, i);
						continue;
					}
				}
			}
		}

		register_class_metadata(self, class_metadata.class_metadata);

		return *this;
	}
}// namespace Engine
