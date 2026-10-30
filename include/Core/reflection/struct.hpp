#pragma once
#include <Core/etl/flat_set.hpp>
#include <Core/reflection/type.hpp>

#include <Core/reflection/struct.generated.hpp>

namespace Trinex::Refl
{
	class Property;
	class Function;

	trinex_reflect()
	class trinex_api Struct : public Type
	{
		trinex_reflection_body();

	public:
		class VTable : public Type::VTable
		{
		};

	private:
		struct Compare
		{
			using is_transparent = void;

			inline bool operator()(Name lhs, Name rhs) const { return lhs.id() < rhs.id(); }
			inline bool operator()(const Ref<Object>& lhs, Name rhs) const { return lhs->name().id() < rhs.id(); }
			inline bool operator()(Name lhs, const Ref<Object>& rhs) const { return lhs.id() < rhs->name().id(); }

			inline bool operator()(const Ref<Object>& lhs, const Ref<Object>& rhs) const
			{
				return lhs->name().id() < rhs->name().id();
			}
		};

	private:
		Array<Ref<Struct>> m_bases;
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

		Struct(Name name, u32 size, u32 alignment, u64 flags = 0, Array<Ref<Struct>> bases = {}, const VTable* vtable = nullptr,
		       Object* owner = nullptr);

		Object* find(Name name, const Class* base = nullptr) const override;
		Object* resolve(const Class* base) override;
		const Object* resolve(const Class* base) const override;

		bool is_child_of(const Struct* other) const;

		Class* reflection() const override { return class_of<Struct>(); }

		inline const Array<Ref<Struct>>& bases() const { return m_bases; }
		inline const Array<Ref<Object>>& members() const { return m_members.container(); }
		inline const VTable* vtable() const { return static_cast<const VTable*>(Type::vtable()); }
		inline bool is_abstract() const { return has_flags(IsAbstract); }
		inline bool is_final() const { return has_flags(IsFinal); }
		inline bool is_standard_layout() const { return has_flags(IsStandardLayout); }
		inline bool is_trivial() const { return has_flags(IsTrivial); }

		using Type::allocate;

		template<typename T>
		bool is_child_of() const
		{
			return is_child_of(struct_of<T>());
		}

		template<typename T, typename... Args>
		T* allocate(Args&&... args) const
		{
			if (is_child_of<T>())
			{
				return static_cast<T*>(allocate(static_cast<Args&&>(args)...));
			}

			return nullptr;
		}
	};
} // namespace Trinex::Refl
