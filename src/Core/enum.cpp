#include <Core/constants.hpp>
#include <Core/enum.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	using EnumMap = Map<String, Enum*>;
	static EnumMap& enums_map()
	{
		static EnumMap enums;
		return enums;
	}

	static void on_destroy()
	{
		for (auto& [name, enum_entry] : enums_map())
		{
			delete enum_entry;
		}

		enums_map().clear();
	}

	static PostDestroyController destroy_struct_map(on_destroy);

	ENGINE_EXPORT Enum* Enum::create(const String& namespace_name, const String& name,
									 const Vector<Enum::Entry>& entries)
	{
		Name full_name = Name(namespace_name.empty() ? name : namespace_name + "::" + name);
		Enum* _enum	   = static_find(full_name);

		info_log("Enum", "Register enum '%s'", full_name.c_str());

		if (!_enum)
		{
			_enum				   = new Enum();
			enums_map()[full_name] = _enum;

			_enum->m_base_name = name;
			_enum->m_full_name = full_name;

			if (!namespace_name.empty())
				_enum->m_namespace_name = namespace_name;

			// Register this enum to script engine
			ScriptEnumRegistrar registrar(namespace_name, name, true);

			for (auto& entry : entries)
			{
				_enum->create_entry(entry.name, entry.value);
			}
		}

		return _enum;
	}

	Index Enum::index_of(const Name& name) const
	{
		auto it = m_entries_by_name.find(name);
		if (it == m_entries_by_name.end())
			return Constants::index_none;
		return it->second;
	}

	Index Enum::index_of(EnumerateType value) const
	{
		auto it = m_entries_by_value.find(value);
		if (it == m_entries_by_value.end())
			return Constants::index_none;
		return it->second;
	}

	const Enum::Entry* Enum::entry(EnumerateType value) const
	{
		Index index = index_of(value);
		return index == Constants::index_none ? nullptr : &(m_entries[index]);
	}

	const Enum::Entry* Enum::entry(const Name& name) const
	{
		Index index = index_of(name);
		return index == Constants::index_none ? nullptr : &(m_entries[index]);
	}

	const Enum::Entry* Enum::create_entry(const Name& name, EnumerateType value)
	{
		if (const Entry* e = entry(name))
			return e;
		if (const Entry* e = entry(value))
			return e;

		Entry new_entry;
		new_entry.name	= name;
		new_entry.value = value;

		Index index = m_entries.size();

		m_entries.push_back(new_entry);
		m_entries_by_name.insert({name, index});
		m_entries_by_value.insert({value, index});

		ScriptEnumRegistrar registrar(m_namespace_name, m_base_name, false);
		registrar.set(name.c_str(), static_cast<int_t>(value));

		return &(m_entries.back());
	}

	const Name& Enum::name() const
	{
		return m_full_name;
	}

	const Name& Enum::namespace_name() const
	{
		return m_namespace_name;
	}

	const Name& Enum::base_name() const
	{
		return m_base_name;
	}

	const Vector<Enum::Entry>& Enum::entries() const
	{
		return m_entries;
	}

	ENGINE_EXPORT Enum* Enum::static_find(const String& name, bool required)
	{
		auto& map = enums_map();
		auto it	  = map.find(name);

		if (it == map.end())
		{
			// Maybe initializer is not executed?
			ReflectionInitializeController().require(name);
			it = map.find(name);

			if (it != map.end())
				return it->second;

			if (required)
			{
				throw EngineException(Strings::format("Failed to find enum '{}'", name.c_str()));
			}
			return nullptr;
		}

		return it->second;
	}
}// namespace Engine
