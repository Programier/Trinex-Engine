#include <Core/reflection/new/container_type.hpp>

namespace Trinex::Refl::New
{
	ContainerType::ContainerType(Name name, u32 size, u32 alignment, u64 flags, const VTable* vtable, Object* owner)
	    : TemplateType(name, size, alignment, flags, vtable, owner)
	{}

	bool ContainerType::size(const void* container, usize& result) const
	{
		if (container == nullptr || vtable() == nullptr)
			return false;

		result = vtable()->size(container);
		return true;
	}

	bool ContainerType::clear(void* container) const
	{
		if (container == nullptr || vtable() == nullptr)
			return false;

		vtable()->clear(container);
		return true;
	}
}// namespace Trinex::Refl::New
