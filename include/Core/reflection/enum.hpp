#pragma once
#include <Core/etl/value_info.hpp>
#include <Core/reflection/object.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine::Refl
{
	class ENGINE_EXPORT Enum : public Object
	{
		declare_reflect_type(Enum, Object);

	public:
		struct Entry {
			Name name;
			EnumerateType value;

			FORCE_INLINE Entry() : name(Name::none), value(0) {}

			template<typename EnumValue = EnumerateType>
			FORCE_INLINE Entry(const Name& name, EnumValue value) : name(name), value(static_cast<EnumerateType>(value))
			{}

			template<typename EnumValue = EnumerateType>
			FORCE_INLINE Entry(EnumValue value, const Name& name) : name(name), value(static_cast<EnumerateType>(value))
			{}
		};

		enum Flags : byte
		{
			Undefined    = 0,
			IsScriptable = BIT(0),
			IsBitFlags   = BIT(1),
		};

	protected:
		TreeMap<Name, Index> m_entries_by_name;
		TreeMap<EnumerateType, Index> m_entries_by_value;
		Vector<Entry> m_entries;
		ScriptTypeInfo m_info;
		byte m_flags;

		const Entry* create_entry(void* registrar, const Name& name, EnumerateType value);
		Enum& register_enum_with_entries(const Vector<Enum::Entry>& entries);
		static StringView extract_enum_value_name(StringView full_name);

	public:
		Enum(byte flags = 0);

		template<typename EnumType, auto... enum_values>
		static Enum* create(const StringView& name, byte flags)
		    requires(std::is_enum_v<EnumType> && sizeof(EnumType) <= sizeof(EnumerateType))
		{
			auto name_of = extract_enum_value_name;
			auto entries = Vector<Enum::Entry>{Entry(name_of(value_info<enum_values>::name()), enum_values)...};
			return &Object::new_instance<Enum>(name, flags)->register_enum_with_entries(entries);
		}

		Index index_of(const Name& name) const;
		Index index_of(EnumerateType value) const;
		const Entry* entry(EnumerateType value) const;
		const Entry* entry(const Name& name) const;
		const Entry* create_entry(const Name& name, EnumerateType value);
		const Vector<Enum::Entry>& entries() const;

		inline bool is_scriptable() const { return m_flags & IsScriptable; }
		inline bool is_bit_flags() const { return m_flags & IsBitFlags; }
	};

#define trinex_implement_enum(enum_name, flags, ...)                                                                             \
	Engine::Refl::Enum* enum_name::s_enum = nullptr;                                                                             \
	void enum_name::static_initialize_enum()                                                                                     \
	{                                                                                                                            \
		s_enum = Engine::Refl::Enum::create<enum_name::Enum, __VA_ARGS__>(#enum_name, flags);                                    \
	}                                                                                                                            \
	static Engine::byte TRINEX_CONCAT(trinex_engine_refl_enum_, __LINE__) = static_cast<Engine::byte>(                           \
	        Engine::ReflectionInitializeController([]() { enum_name::static_initialize_enum(); }, #enum_name).id())

#define trinex_implement_engine_enum(enum_name, flags, ...) trinex_implement_enum(Engine::enum_name, flags, __VA_ARGS__)
}// namespace Engine::Refl
