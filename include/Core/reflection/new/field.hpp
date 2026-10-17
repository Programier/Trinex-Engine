#pragma once
#include <Core/reflection/new/object.hpp>


namespace Trinex::Refl::New
{
	class Type;

	class Field : public Object
	{
	private:
		const Type* m_type;

	public:
		enum Flags : u64
		{
			IsConst     = 1ull << 0,
			IsPointer   = 1ull << 1,
			IsReference = 1ull << 2,
		};

		Field(Name name, const Type* type, u64 flags = 0);

		inline const Type* type() const { return m_type; }
		inline bool is_const() const { return has_flags(IsConst); }
		inline bool is_pointer() const { return has_flags(IsPointer); }
		inline bool is_reference() const { return has_flags(IsReference); }
	};
}// namespace Trinex::Refl::New
