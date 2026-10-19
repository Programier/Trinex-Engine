#pragma once

#include <Core/reflection/new/container_type.hpp>

namespace Trinex::Refl::New
{
	class trinex_api ArrayType : public ContainerType
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

			void construct(void* dst, usize count = 1) const override;
			void destruct(void* value, usize count = 1) const override;
			void copy_construct(void* dst, const void* src, usize count = 1) const override;
			void move_construct(void* dst, void* src, usize count = 1) const override;
			void copy_assign(void* dst, const void* src, usize count = 1) const override;
			void move_assign(void* dst, void* src, usize count = 1) const override;
			void swap(void* lhs, void* rhs, usize count = 1) const override;
			void relocate(void* dst, void* src, usize count = 1) const override;

			bool equals(const void* lhs, const void* rhs) const override;
			i32 compare(const void* lhs, const void* rhs) const override;

			usize size(const void* container) const override;
			void clear(void* container) const override;

			inline Type* argument() const { return m_argument.value(); }
		};

	private:
		VTable m_vtable;

	public:
		ArrayType(Name name, Type* argument, Object* owner = nullptr);

		Type* argument(usize idx) const override;

		inline Type* argument() const { return m_vtable.argument(); }
	};
}// namespace Trinex::Refl::New
