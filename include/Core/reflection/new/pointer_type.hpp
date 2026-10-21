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

		static PointerType* from(Type* type);
		static const VTable* vtable();

		inline Type* type() const { return m_type.value(); }
	};
}// namespace Trinex::Refl::New
