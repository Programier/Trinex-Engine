#pragma once
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	class trinex_api PointerType : public Type
	{
	public:
		struct VTable : public Type::VTable {
		};

	private:
		Ref<Type> m_type;

	public:
		PointerType(Name name, Type* type, Object* owner = nullptr);

		inline Type* type() const { return m_type.value(); }
		inline const VTable* vtable() const { return static_cast<const VTable*>(Type::vtable()); }
	};
}// namespace Trinex::Refl::New
