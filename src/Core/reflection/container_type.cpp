#include <Core/reflection/container_type.hpp>

namespace Trinex::Refl
{
	ContainerType::ContainerType(Name name, u32 size, u32 alignment, u64 flags, const VTable* vtable, Object* owner)
	    : TemplateType(name, size, alignment, flags, vtable, owner)
	{}

	bool ContainerType::size(const void* container, usize& result) const
	{
		if (container == nullptr || vtable()->size == nullptr)
			return false;

		result = vtable()->size(this, container);
		return true;
	}

	bool ContainerType::clear(void* container) const
	{
		if (container == nullptr || vtable()->clear == nullptr)
			return false;

		vtable()->clear(this, container);
		return true;
	}
} // namespace Trinex::Refl
