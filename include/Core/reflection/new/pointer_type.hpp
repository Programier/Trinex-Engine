#pragma once
#include <Core/reflection/new/type.hpp>

#include <Core/reflection/new/pointer_type.generated.hpp>

namespace Trinex::Refl::New
{
	trinex_reflect()
	class trinex_api PointerType : public Type
	{
		trinex_reflection_body();

	public:
		struct VTable : public Type::VTable
		{
		};

	private:
		Ref<Type> m_type;

	public:
		PointerType(Name name, Type* type, Object* owner = nullptr);

		static PointerType* from(Type* type);
		static const VTable* vtable();

		Object* resolve(const Class* base) override;
		const Object* resolve(const Class* base) const override;
		Class* reflection() const override { return class_of<PointerType>(); }

		inline Type* type() const { return m_type.value(); }
	};
} // namespace Trinex::Refl::New
