#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	Type::Type(Name name, u32 size, u32 alignment, u64 flags, const VTable* vtable, Object* owner)
	    : Object(name, flags, owner), m_size(size), m_alignment(alignment), m_vtable(vtable)
	{
		trinex_verify(vtable);
	}

	bool Type::construct(void* dst, usize count) const
	{
		if (m_vtable->construct == nullptr || dst == nullptr)
			return false;
		m_vtable->construct(this, dst, count);
		return true;
	}

	bool Type::destruct(void* value, usize count) const
	{
		if (m_vtable->destruct == nullptr || value == nullptr)
			return false;
		m_vtable->destruct(this, value, count);
		return true;
	}

	bool Type::copy_construct(void* dst, const void* src, usize count) const
	{
		if (m_vtable->copy_construct == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->copy_construct(this, dst, src, count);
		return true;
	}

	bool Type::move_construct(void* dst, void* src, usize count) const
	{
		if (m_vtable->move_construct == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->move_construct(this, dst, src, count);
		return true;
	}

	bool Type::copy_assign(void* dst, const void* src, usize count) const
	{
		if (m_vtable->copy_assign == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->copy_assign(this, dst, src, count);
		return true;
	}

	bool Type::move_assign(void* dst, void* src, usize count) const
	{
		if (m_vtable->move_assign == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->move_assign(this, dst, src, count);
		return true;
	}

	bool Type::swap(void* lhs, void* rhs, usize count) const
	{
		if (m_vtable->swap == nullptr || lhs == nullptr || rhs == nullptr)
			return false;
		m_vtable->swap(this, lhs, rhs, count);
		return true;
	}

	bool Type::relocate(void* dst, void* src, usize count) const
	{
		if (m_vtable->relocate == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->relocate(this, dst, src, count);
		return true;
	}

	bool Type::equals(const void* lhs, const void* rhs) const
	{
		return m_vtable != nullptr && m_vtable->equals != nullptr && lhs != nullptr && rhs != nullptr &&
		       m_vtable->equals(this, lhs, rhs);
	}

	bool Type::compare(const void* lhs, const void* rhs, i32& result) const
	{
		if (m_vtable->compare == nullptr || lhs == nullptr || rhs == nullptr)
			return false;
		result = m_vtable->compare(this, lhs, rhs);
		return true;
	}

	bool Type::parse(void* self, StringView value) const
	{
		if (m_vtable->parse == nullptr || self == nullptr)
			return false;

		return m_vtable->parse(this, self, value);
	}

	bool Type::format(const void* self, String& value) const
	{
		if (m_vtable->format == nullptr || self == nullptr)
			return false;

		return m_vtable->format(this, self, value);
	}
}// namespace Trinex::Refl::New
