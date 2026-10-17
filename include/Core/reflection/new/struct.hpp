#pragma once

#include <Core/etl/flat_set.hpp>
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	class Property;
	class Function;

	class Struct : public Type
	{
	public:
		class VTable : public Type::VTable
		{
		};

	private:
		struct Compare {
			using is_transparent = void;

			inline bool operator()(Name lhs, Name rhs) const { return lhs.id() < rhs.id(); }
			inline bool operator()(const Ref<Object>& lhs, Name rhs) const { return lhs->name().id() < rhs.id(); }
			inline bool operator()(Name lhs, const Ref<Object>& rhs) const { return lhs.id() < rhs->name().id(); }

			inline bool operator()(const Ref<Object>& lhs, const Ref<Object>& rhs) const
			{
				return lhs->name().id() < rhs->name().id();
			}
		};

		Ref<const Struct> m_base;
		FlatSet<Ref<Object>, Compare> m_members;

	private:
		bool register_child(Object* object) override;
		bool unregister_child(Object* object) override;

	public:
		enum Flags : u64
		{
			IsAbstract       = 1ull << 8,
			IsFinal          = 1ull << 9,
			IsStandardLayout = 1ull << 10,
			IsTrivial        = 1ull << 11,
		};

		Struct(Name name, u32 size, u32 alignment, u64 flags = 0, const Struct* base = nullptr, const VTable* vtable = nullptr,
		       Object* owner = nullptr);

		Object* find(Name name) const override;
		bool is_a(const Struct* other) const;

		inline const Struct* base() const { return m_base.value(); }
		inline const Vector<Ref<Object>>& members() const { return m_members.container(); }
		inline const VTable* vtable() const { return static_cast<const VTable*>(Type::vtable()); }
		inline bool is_abstract() const { return has_flags(IsAbstract); }
		inline bool is_final() const { return has_flags(IsFinal); }
		inline bool is_standard_layout() const { return has_flags(IsStandardLayout); }
		inline bool is_trivial() const { return has_flags(IsTrivial); }
	};
}// namespace Trinex::Refl::New
