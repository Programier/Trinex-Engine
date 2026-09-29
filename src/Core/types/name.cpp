#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/templates.hpp>
#include <Core/etl/vector.hpp>
#include <Core/memory.hpp>
#include <Core/types/name.hpp>
#include <ScriptEngine/script_binding.hpp>
#include <cstring>

namespace Trinex
{

#define declare_custom_name(var_name, name) const Name Name::var_name = #name
#define declare_name(name) declare_custom_name(name, name)
	declare_name(undefined);
	declare_name(out_of_range);
	declare_name(model);
	declare_name(texture);
	declare_name(color);
	declare_name(offset);
	declare_name(ambient_color);
	declare_name(radius);
	declare_name(fall_off_exponent);
	declare_name(height);
	declare_name(spot_angles);
	declare_name(inverse_rotation);
	declare_name(scale);
	declare_name(intensivity);
	declare_name(location);
	declare_name(direction);
	declare_name(mask);

	static Vector<Name::Entry>& name_entries()
	{
		static Vector<Name::Entry> entries;
		return entries;
	}

	static Map<u64, u32>& name_index_map()
	{
		static Map<u64, u32> indices;
		return indices;
	}

	static const String& default_string()
	{
		static String default_name;
		return default_name;
	}

	static FORCE_INLINE void push_new_name(const char* name, usize len, u64 hash)
	{
		Map<u64, u32>& indices = name_index_map();
		const u32 index        = static_cast<u32>(name_entries().size());
		const auto it          = indices.find(hash);
		const u32 next         = it == indices.end() ? 0xFFFFFFFF : it->second;

		if (it == indices.end())
		{
			indices.insert({hash, index});
		}
		else
		{
			it->second = index;
		}

		name_entries().push_back(Name::Entry{String(name, len), hash, next});
	}

	ENGINE_EXPORT Name Name::none;

	static constexpr u32 invalid_name_index = 0xFFFFFFFF;
	static constexpr usize cache_size       = 64;

	struct NameCacheEntry {
		const char* data = nullptr;
		usize length     = 0;
		u64 hash         = 0;
		u32 index        = invalid_name_index;
	};

	struct NameCache {
		NameCacheEntry by_data[cache_size];
		NameCacheEntry by_hash[cache_size];
	};

	static NameCache& name_cache()
	{
		static thread_local NameCache cache;
		return cache;
	}

	static FORCE_INLINE usize data_cache_slot(const StringView& view)
	{
		return ((reinterpret_cast<usize>(view.data()) >> 4) ^ view.length()) & (cache_size - 1);
	}

	static FORCE_INLINE usize hash_cache_slot(u64 hash, usize length)
	{
		return (hash ^ (hash >> 32) ^ length) & (cache_size - 1);
	}

	static FORCE_INLINE bool cached_name_matches(const NameCacheEntry& cache, const StringView& view)
	{
		if (cache.index == invalid_name_index || cache.length != view.length() || cache.index >= name_entries().size())
		{
			return false;
		}

		const String& string = name_entries()[cache.index].name;
		return string.length() == view.length() && std::memcmp(string.data(), view.data(), view.length()) == 0;
	}

	static FORCE_INLINE bool find_cached_name(const StringView& view, u32& index)
	{
		NameCacheEntry& cache = name_cache().by_data[data_cache_slot(view)];

		if (cache.data == view.data() && cached_name_matches(cache, view))
		{
			index = cache.index;
			return true;
		}

		return false;
	}

	static FORCE_INLINE bool find_cached_name(const StringView& view, u64 hash, u32& index)
	{
		NameCacheEntry& cache = name_cache().by_hash[hash_cache_slot(hash, view.length())];

		if (cache.hash == hash && cached_name_matches(cache, view))
		{
			index = cache.index;
			return true;
		}

		return false;
	}

	static FORCE_INLINE void cache_name(const StringView& view, u64 hash, u32 index)
	{
		NameCacheEntry cache{view.data(), view.length(), hash, index};
		name_cache().by_data[data_cache_slot(view)]                = cache;
		name_cache().by_hash[hash_cache_slot(hash, view.length())] = cache;
	}

	static FORCE_INLINE bool find_name_index(const StringView& view, u64 hash, u32& out_index)
	{
		Vector<Name::Entry>& name_table = name_entries();
		Map<u64, u32>& indices          = name_index_map();
		const auto head                 = indices.find(hash);

		if (head == indices.end())
		{
			return false;
		}

		for (u32 index = head->second; index != invalid_name_index; index = name_table[index].next)
		{
			const Name::Entry& entry = name_table[index];

			if (entry.name == view)
			{
				out_index = index;
				return true;
			}
		}

		return false;
	}

	Name& Name::assign(const StringView& view)
	{
		if (view.empty())
		{
			m_id = invalid_name_index;
			return *this;
		}

		if (find_cached_name(view, m_id))
		{
			return *this;
		}

		u64 hash                        = memory_hash(view.data(), view.length(), 0);
		Vector<Name::Entry>& name_table = name_entries();

		if (find_cached_name(view, hash, m_id))
		{
			return *this;
		}

		if (find_name_index(view, hash, m_id))
		{
			cache_name(view, hash, m_id);
			return *this;
		}

		m_id = static_cast<u32>(name_table.size());
		push_new_name(view.data(), view.length(), hash);
		cache_name(view, hash, m_id);
		return *this;
	}

	u64 Name::hash() const
	{
		return is_valid() ? name_entries()[m_id].hash : Constants::invalid_hash;
	}

	bool Name::equals(const StringView& name) const
	{
		if (is_valid())
		{
			const String& str = name_entries()[m_id].name;
			return str == name;
		}

		return false;
	}

	const Name& Name::to_string(String& out) const
	{
		if (is_valid())
		{
			out += name_entries()[m_id].name;
		}

		return *this;
	}

	const String& Name::to_string() const
	{
		if (is_valid())
		{
			return name_entries()[m_id].name;
		}

		return default_string();
	}

	bool Name::serialize(class Archive& ar)
	{
		bool valid = is_valid();
		ar.serialize(valid);

		if (valid)
		{
			String string_name = to_string();
			ar.serialize(string_name);

			if (ar.is_reading())
			{
				(*this) = string_name;
			}
		}

		return ar;
	}

	trinex_on_pre_init({.name = "Trinex::Name", .after = {"Trinex::StringView"}})
	{
		auto flags = ScriptClassFlags::Pod | ScriptClassFlags::AppClassAllInts | ScriptClassFlags::AppClassAlign8 |
		             ScriptClassFlags::AppClassMoreConstructors;
		ScriptBinding::Class registrar = ScriptBinding::Class::create("Trinex::Name", ScriptBinding::value_type<Name>(flags));

		registrar.behaviour(ScriptClassBehave::Construct, "void f()", ScriptBinding::Helpers::constructor<Name>,
		                    ScriptCallConv::CDeclObjFirst);
		registrar.behaviour(ScriptClassBehave::Construct, "void f(const string&)",
		                    ScriptBinding::Helpers::constructor<Name, const String&>, ScriptCallConv::CDeclObjFirst);
		registrar.behaviour(ScriptClassBehave::Construct, "void f(const StringView&)",
		                    ScriptBinding::Helpers::constructor<Name, const StringView&>, ScriptCallConv::CDeclObjFirst);

		registrar.method("bool is_valid() const", &Name::is_valid);
		registrar.method("uint64 hash() const", &Name::hash);
		registrar.method("const string& to_string() const", overload_of<const String&()>(&Name::to_string));
		registrar.method("const Name& to_string(string&) const", overload_of<const Name&()>(&Name::to_string));

		registrar.method("Trinex::Name& opAssign(const StringView&)", overload_of<Name&(const StringView&)>(&Name::operator=));
		registrar.method("Trinex::Name& opAssign(const string&)", overload_of<Name&(const String&)>(&Name::operator=));
		registrar.method("bool opEquals(const StringView&) const", overload_of<bool(const StringView&)>(&Name::operator==));
		registrar.method("bool opEquals(const string&) const", overload_of<bool(const String&)>(&Name::operator==));
		registrar.method("bool opEquals(const Name&) const", overload_of<bool(const Name&)>(&Name::operator==));

		registrar.method("const string& opConv() const", &Name::operator const std::basic_string<char>&);
		registrar.method("const string& opImplConv() const", &Name::operator const std::basic_string<char>&);
	}
}// namespace Trinex
