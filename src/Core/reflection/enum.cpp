#include <Core/constants.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>

namespace Engine::Refl
{
	implement_reflect_type(Enum);

	Enum::Enum(byte flags) : m_flags(flags) {}

	Enum& Enum::register_enum_with_entries(const Vector<Enum::Entry>& entries)
	{
		String name = full_name();
		info_log("Enum", "Register enum '%s'", name.c_str());

		if (m_flags & IsScriptable)
		{
			ScriptEnumRegistrar registrar(name, true);
			m_info = registrar.type_info();

			for (auto& entry : entries)
			{
				create_entry(&registrar, entry.name, entry.value);
			}
		}
		else
		{
			for (auto& entry : entries)
			{
				create_entry(nullptr, entry.name, entry.value);
			}
		}
		return *this;
	}

	StringView Enum::extract_enum_value_name(StringView full_name)
	{
		return Strings::class_name_sv_of(full_name);
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

	const Enum::Entry* Enum::create_entry(void* registrar_ptr, const Name& name, EnumerateType value)
	{
		if (const Entry* e = entry(name))
			return e;
		if (const Entry* e = entry(value))
			return e;

		Entry new_entry;
		new_entry.name  = name;
		new_entry.value = value;

		Index index = m_entries.size();

		m_entries.push_back(new_entry);
		m_entries_by_name.insert({name, index});
		m_entries_by_value.insert({value, index});

		if (registrar_ptr)
		{
			ScriptEnumRegistrar* registrar = reinterpret_cast<ScriptEnumRegistrar*>(registrar_ptr);
			registrar->set(name.c_str(), static_cast<int_t>(value));
		}

		return &m_entries.back();
	}

	const Enum::Entry* Enum::create_entry(const Name& name, EnumerateType value)
	{
		ScriptEnumRegistrar registrar(scope_name(), this->name(), false);
		return create_entry(&registrar, name, value);
	}

	const Vector<Enum::Entry>& Enum::entries() const
	{
		return m_entries;
	}
}// namespace Engine::Refl
