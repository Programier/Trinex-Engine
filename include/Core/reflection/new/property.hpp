#pragma once
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	class Type;

	class Property : public Object
	{
	public:
		enum Flags : u64
		{
			IsConst      = 1ull << 0,
			IsPointer    = 1ull << 1,
			IsReference  = 1ull << 2,
			IsReadOnly   = 1ull << 3,
			IsTransient  = 1ull << 4,
			IsSerialized = 1ull << 5,
			IsReplicated = 1ull << 6,
			IsThis       = 1ull << 7,
		};

	private:
		Ref<Type> m_type;

	public:
		Property(Name name, Type* type, u64 flags = 0, Object* owner = nullptr);

		virtual void* address(void* context)                                     = 0;
		virtual const void* load(const void* context, void* temporary = nullptr) = 0;
		virtual void store(void* context, void* object)                          = 0;

		inline const Type* type() const { return m_type.value(); }
		inline bool is_const() const { return has_flags(IsConst); }
		inline bool is_pointer() const { return has_flags(IsPointer); }
		inline bool is_reference() const { return has_flags(IsReference); }
		inline bool is_read_only() const { return has_flags(IsReadOnly); }
		inline bool is_transient() const { return has_flags(IsTransient); }
		inline bool is_serialized() const { return has_flags(IsSerialized); }
		inline bool is_replicated() const { return has_flags(IsReplicated); }
		inline bool is_this() const { return has_flags(IsThis); }
	};

}// namespace Trinex::Refl::New
