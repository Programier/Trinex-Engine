#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	Type::Type(Name name, u32 size, u32 alignment, u64 flags, const VTable* vtable, Object* owner)
	    : Object(name, flags, owner), m_size(size), m_alignment(alignment), m_vtable(vtable)
	{}

	bool Type::construct(void* dst, usize count) const
	{
		if (m_vtable == nullptr || dst == nullptr)
			return false;
		m_vtable->construct(dst, count);
		return true;
	}

	bool Type::destruct(void* value, usize count) const
	{
		if (m_vtable == nullptr || value == nullptr)
			return false;
		m_vtable->destruct(value, count);
		return true;
	}

	bool Type::copy_construct(void* dst, const void* src, usize count) const
	{
		if (m_vtable == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->copy_construct(dst, src, count);
		return true;
	}

	bool Type::move_construct(void* dst, void* src, usize count) const
	{
		if (m_vtable == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->move_construct(dst, src, count);
		return true;
	}

	bool Type::copy_assign(void* dst, const void* src, usize count) const
	{
		if (m_vtable == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->copy_assign(dst, src, count);
		return true;
	}

	bool Type::move_assign(void* dst, void* src, usize count) const
	{
		if (m_vtable == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->move_assign(dst, src, count);
		return true;
	}

	bool Type::swap(void* lhs, void* rhs, usize count) const
	{
		if (m_vtable == nullptr || lhs == nullptr || rhs == nullptr)
			return false;
		m_vtable->swap(lhs, rhs, count);
		return true;
	}

	bool Type::relocate(void* dst, void* src, usize count) const
	{
		if (m_vtable == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->relocate(dst, src, count);
		return true;
	}

	bool Type::equals(const void* lhs, const void* rhs) const
	{
		return m_vtable != nullptr && lhs != nullptr && rhs != nullptr && m_vtable->equals(lhs, rhs);
	}

	bool Type::compare(const void* lhs, const void* rhs, i32& result) const
	{
		if (m_vtable == nullptr || lhs == nullptr || rhs == nullptr)
			return false;
		result = m_vtable->compare(lhs, rhs);
		return true;
	}
}// namespace Trinex::Refl::New
