#pragma once
#include <Core/reflection/new/field.hpp>


namespace Trinex::Refl::New
{
	class Property : public Field
	{
	public:
		enum Flags : u64
		{
			IsReadOnly   = 1ull << 8,
			IsTransient  = 1ull << 9,
			IsSerialized = 1ull << 10,
			IsReplicated = 1ull << 11,
		};

		Property(Name name, const Type* type, u64 flags = 0);
		inline bool is_read_only() const { return has_flags(IsReadOnly); }
		inline bool is_transient() const { return has_flags(IsTransient); }
		inline bool is_serialized() const { return has_flags(IsSerialized); }
		inline bool is_replicated() const { return has_flags(IsReplicated); }
	};
}// namespace Trinex::Refl::New
