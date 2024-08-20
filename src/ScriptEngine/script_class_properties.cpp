#include <Core/class.hpp>
#include <Core/property.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>
#include <regex>


namespace Engine
{
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

	template<typename T, PropertyType prop_type>
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

			if (!ScriptPrimitivePropBase::is_const() && object && property_value.type() == prop_type)
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

		String name	  = String(self->script_type_info.property_name(prop_idx));
		String desc	  = "";
		String group  = "";
		BitMask flags = 0;

		parse_metadata(metadata, name, desc, group, flags);

		Property* prop = nullptr;

		if (ScriptEngine::is_bool(type_id))
		{
			prop = new ScriptPrimitiveProp<bool, PropertyType::Bool>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_int8(type_id))
		{
			prop = new ScriptPrimitiveProp<int8_t, PropertyType::Int8>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_int16(type_id))
		{
			prop = new ScriptPrimitiveProp<int16_t, PropertyType::Int16>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_int32(type_id))
		{
			prop = new ScriptPrimitiveProp<int32_t, PropertyType::Int32>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_int64(type_id))
		{
			prop = new ScriptPrimitiveProp<int64_t, PropertyType::Int64>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_uint8(type_id))
		{
			prop = new ScriptPrimitiveProp<uint8_t, PropertyType::UnsignedInt8>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_uint16(type_id))
		{
			prop = new ScriptPrimitiveProp<uint16_t, PropertyType::UnsignedInt16>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_uint32(type_id))
		{
			prop = new ScriptPrimitiveProp<uint32_t, PropertyType::UnsignedInt32>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_uint64(type_id))
		{
			prop = new ScriptPrimitiveProp<uint64_t, PropertyType::UnsignedInt64>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_float(type_id))
		{
			prop = new ScriptPrimitiveProp<float, PropertyType::Float>(offset, name, desc, group, flags);
		}
		else if (ScriptEngine::is_double(type_id))
		{
			prop = new ScriptPrimitiveProp<double, PropertyType::Double>(offset, name, desc, group, flags);
		}

		self->add_property(prop);
	}

	Script& Script::register_properties(Class* self)
	{
		auto& type		= self->script_type_info;
		auto prop_count = type.property_count();

		if (prop_count > 0)
		{
			String type_name	 = Strings::concat_scoped_name(type.namespace_name(), type.name());
			auto& class_metadata = metadata_for_class(type_name);

			for (uint_t i = 0; i < prop_count; ++i)
			{
				if (!type.is_property_native(i))
				{
					auto prop_type_id = type.property_type_id(i);
					auto& metadata	  = class_metadata.metadata_for_property(i);

					if (ScriptEngine::is_primitive_type(prop_type_id))
					{
						register_primitive_property(self, metadata, prop_type_id, i);
						continue;
					}
				}
			}
		}

		return *this;
	}
}// namespace Engine
