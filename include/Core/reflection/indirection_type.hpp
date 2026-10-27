#pragma once
#include <Core/reflection/type.hpp>

#include <Core/reflection/indirection_type.generated.hpp>

namespace Trinex::Refl
{
	trinex_reflect()
	class trinex_api IndirectionType : public Type
	{
		trinex_reflection_body();

	public:
		struct VTable : public Type::VTable
		{
		};

	private:
		Ref<Type> m_type;

	public:
		IndirectionType(Name name, Type* type, Object* owner = nullptr);

		static IndirectionType* from(Type* type, bool rvalue = false);
		static const VTable* vtable();

		Object* resolve(const Class* base) override;
		const Object* resolve(const Class* base) const override;
		Class* reflection() const override { return class_of<IndirectionType>(); }

		inline Type* type() const { return m_type.value(); }
	};
} // namespace Trinex::Refl
