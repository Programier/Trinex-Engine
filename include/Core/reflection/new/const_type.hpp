#pragma once
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	trinex_reflect() class trinex_api ConstType : public Type
	{
	private:
		Ref<Type> m_type;

	public:
		ConstType(Name name, Type* type, Object* owner = nullptr);

		static ConstType* from(Type* type);
		Object* find(Name name, const Class* base = nullptr) const override;
		Object* resolve(const Class* base) override;
		const Object* resolve(const Class* base) const override;

		Class* reflection() const override { return class_of<ConstType>(); }
		inline Type* type() const { return m_type.value(); }
	};
}// namespace Trinex::Refl::New
