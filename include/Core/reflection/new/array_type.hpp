#pragma once

#include <Core/reflection/new/container_type.hpp>

namespace Trinex::Refl::New
{
	trinex_reflect() class trinex_api ArrayType : public ContainerType
	{
	public:
		class VTable : public ContainerType::VTable
		{
		private:
			Ref<Type> m_argument;

		public:
			VTable(Type* argument);

			trinex_default_copyable(VTable);
			trinex_default_moveable(VTable);

			inline Type* argument() const { return m_argument.value(); }
		};

	private:
		VTable m_vtable;

	public:
		ArrayType(Name name, Type* argument, Object* owner = nullptr);

		static ArrayType* from(Type* type);

		Type* argument(usize idx) const override;

		Class* reflection() const override { return class_of<ArrayType>(); }

		inline Type* argument() const { return m_vtable.argument(); }
	};
}// namespace Trinex::Refl::New
