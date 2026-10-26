#pragma once
#include <Core/reflection/new/type.hpp>

#include <Core/reflection/new/property.generated.hpp>

namespace Trinex::Refl::New
{
	class Type;

	trinex_reflect() class trinex_api Property : public Object
	{
		trinex_reflection_body();

	public:
		enum Flags : u64
		{
			IsReadOnly   = 1ull << 0,
			IsTransient  = 1ull << 1,
			IsReplicated = 1ull << 2,
			IsThis       = 1ull << 3,
		};

	private:
		Ref<Type> m_type;

	public:
		Property(Name name, Type* type, u64 flags = 0, Object* owner = nullptr);

		virtual void* address(void* context)                                     = 0;
		virtual const void* load(const void* context, void* temporary = nullptr) = 0;
		virtual void store(void* context, void* object)                          = 0;

		Object* resolve(const Class* base) override;
		const Object* resolve(const Class* base) const override;

		Class* reflection() const override { return class_of<Property>(); }

		inline const Type* type() const { return m_type.value(); }
		inline bool is_read_only() const { return has_flags(IsReadOnly); }
		inline bool is_transient() const { return has_flags(IsTransient); }
		inline bool is_replicated() const { return has_flags(IsReplicated); }
		inline bool is_this() const { return has_flags(IsThis); }
	};

}// namespace Trinex::Refl::New
