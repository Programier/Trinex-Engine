#pragma once
#include <Core/reflection/type.hpp>

#include <Core/reflection/template_type.generated.hpp>

namespace Trinex::Refl
{
	trinex_reflect()
	class trinex_api TemplateType : public Type
	{
		trinex_reflection_body();

	public:
		class VTable : public Type::VTable
		{
		};

	public:
		TemplateType(Name name, u32 size, u32 alignment, u64 flags = 0, const VTable* vtable = nullptr, Object* owner = nullptr);

		virtual usize argument_count() const;
		virtual Type* argument(usize index) const;

		Class* reflection() const override { return class_of<TemplateType>(); }

		inline const VTable* vtable() const { return static_cast<const VTable*>(Type::vtable()); }
	};
} // namespace Trinex::Refl
