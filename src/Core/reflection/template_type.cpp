#include <Core/reflection/template_type.hpp>

namespace Trinex::Refl
{
	TemplateType::TemplateType(Name name, u32 size, u32 alignment, u64 flags, const VTable* vtable, Object* owner)
	    : Type(name, size, alignment, flags, vtable)
	{
		if (owner)
		{
			Object::owner(owner);
		}
	}

	usize TemplateType::argument_count() const
	{
		return 0;
	}

	Type* TemplateType::argument(usize index) const
	{
		return nullptr;
	}
} // namespace Trinex::Refl
