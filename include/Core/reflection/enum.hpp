#pragma once
#include <Core/etl/type_info.hpp>
#include <Core/etl/value_info.hpp>
#include <Core/reflection/object.hpp>

namespace Engine::Refl
{
	class ENGINE_EXPORT Enum final : public Object
	{
		declare_reflect_type(Enum, Object);

	public:
		struct Entry {
			Name name;
			EnumerateType value;

			FORCE_INLINE Entry() : name(Name::none), value(0)
			{}

			template<typename EnumValue = EnumerateType>
			FORCE_INLINE Entry(const Name& name, EnumValue value) : name(name), value(static_cast<EnumerateType>(value))
			{}

			template<typename EnumValue = EnumerateType>
			FORCE_INLINE Entry(EnumValue value, const Name& name) : name(name), value(static_cast<EnumerateType>(value))
			{}
		};

		TreeMap<Name, Index> m_entries_by_name;
		TreeMap<EnumerateType, Index> m_entries_by_value;
		Vector<Entry> m_entries;
		Identifier m_id;

		const Entry* create_entry(void* registrar, const Name& name, EnumerateType value);
		static StringView extract_enum_value_name(StringView full_name);
		static Enum* create_internal(const StringView& name, const Vector<Enum::Entry>& entries, Identifier id);

	public:
		Enum(const Vector<Enum::Entry>& entries, Identifier id);

		template<typename EnumType, auto... enum_values>
		static Enum* create(const StringView& name)
		{
			auto name_of = extract_enum_value_name;
			return create_internal(name, {Entry(name_of(value_info<enum_values>::name()), enum_values)...},
								   type_info<EnumType>::id());
		}

		Index index_of(const Name& name) const;
		Index index_of(EnumerateType value) const;
		const Entry* entry(EnumerateType value) const;
		const Entry* entry(const Name& name) const;
		const Entry* create_entry(const Name& name, EnumerateType value);

		const Vector<Enum::Entry>& entries() const;

		~Enum();

		FORCE_INLINE Identifier id() const
		{
			return m_id;
		}
	};

#define implement_enum(enum_name, ...)                                                                                           \
	static Engine::byte TRINEX_CONCAT(trinex_engine_refl_enum_, __LINE__) = static_cast<Engine::byte>(                           \
			Engine::ReflectionInitializeController([]() { Engine::Refl::Enum::create<enum_name, __VA_ARGS__>(#enum_name); },     \
												   #enum_name)                                                                   \
					.id())

#define implement_engine_enum(enum_name, ...) implement_enum(Engine::enum_name, __VA_ARGS__)
}// namespace Engine::Refl
