#pragma once
#include <Core/reflection/new/template_type.hpp>

#include <Core/reflection/new/container_type.generated.hpp>

namespace Trinex::Refl::New
{
	trinex_reflect() class trinex_api ContainerType : public TemplateType
	{
		trinex_reflection_body();

	public:
		struct VTable : public TemplateType::VTable {
			usize (*size)(const ContainerType* type, const void* container) = nullptr;
			void (*clear)(const ContainerType* type, void* container)       = nullptr;
		};

	public:
		ContainerType(Name name, u32 size, u32 alignment, u64 flags = 0, const VTable* vtable = nullptr, Object* owner = nullptr);

		bool size(const void* container, usize& result) const;
		bool clear(void* container) const;

		Class* reflection() const override { return class_of<ContainerType>(); }

		inline const VTable* vtable() const { return static_cast<const VTable*>(Type::vtable()); }
	};
}// namespace Trinex::Refl::New
