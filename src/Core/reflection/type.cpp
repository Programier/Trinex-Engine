#include <Core/reflection/function.hpp>
#include <Core/reflection/function_set.hpp>
#include <Core/reflection/type.hpp>

namespace Trinex::Refl
{
	Type::Type(Name name, u32 size, u32 alignment, u64 flags, const VTable* vtable, Object* owner)
	    : Object(name, flags), m_size(size), m_alignment(alignment), m_vtable(vtable)
	{
		trinex_verify(vtable);

		if (owner)
		{
			Object::owner(owner);
		}
	}

	bool Type::invoke_construct_internal(void* mem, Type* const* args, usize count, void* context) const
	{
		if (mem == nullptr || context == nullptr)
			return false;

		if (count == 0 && m_vtable->construct)
		{
			m_vtable->construct(this, mem);
			return true;
		}

		if (count == 1 && args[0] == type_of<const void*>() && m_vtable->copy_construct)
		{
			constexpr usize offset = (sizeof(void*) + alignof(void*) - 1) & ~(alignof(void*) - 1);
			const void* src        = *reinterpret_cast<const void* const*>(static_cast<unsigned char*>(context) + offset);
			m_vtable->copy_construct(this, mem, src);
			return true;
		}

		if (count == 1 && args[0] == type_of<void*>() && m_vtable->move_construct)
		{
			constexpr usize offset = (sizeof(void*) + alignof(void*) - 1) & ~(alignof(void*) - 1);
			void* src              = *reinterpret_cast<void**>(static_cast<unsigned char*>(context) + offset);
			m_vtable->move_construct(this, mem, src);
			return true;
		}

		static const Name name          = "$constructor";
		const FunctionSet* constructors = find<FunctionSet>(name);
		if (constructors == nullptr)
			return false;

		Function* constructor = constructors->resolve(args, count);
		if (constructor == nullptr)
			return false;

		constructor->invoke(context);
		return true;
	}

	bool Type::construct(void* value) const
	{
		if (m_vtable->construct == nullptr || value == nullptr)
			return false;
		m_vtable->construct(this, value);
		return true;
	}

	bool Type::destruct(void* value) const
	{
		if (m_vtable->destruct == nullptr || value == nullptr)
			return false;
		m_vtable->destruct(this, value);
		return true;
	}

	bool Type::copy_construct(void* dst, const void* src) const
	{
		if (m_vtable->copy_construct == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->copy_construct(this, dst, src);
		return true;
	}

	bool Type::move_construct(void* dst, void* src) const
	{
		if (m_vtable->move_construct == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->move_construct(this, dst, src);
		return true;
	}

	bool Type::copy_assign(void* dst, const void* src) const
	{
		if (m_vtable->copy_assign == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->copy_assign(this, dst, src);
		return true;
	}

	bool Type::move_assign(void* dst, void* src) const
	{
		if (m_vtable->move_assign == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->move_assign(this, dst, src);
		return true;
	}

	bool Type::swap(void* lhs, void* rhs) const
	{
		if (m_vtable->swap == nullptr || lhs == nullptr || rhs == nullptr)
			return false;
		m_vtable->swap(this, lhs, rhs);
		return true;
	}

	bool Type::relocate(void* dst, void* src) const
	{
		if (m_vtable->relocate == nullptr || dst == nullptr || src == nullptr)
			return false;
		m_vtable->relocate(this, dst, src);
		return true;
	}

	bool Type::equals(const void* lhs, const void* rhs) const
	{
		return m_vtable != nullptr && m_vtable->equals != nullptr && lhs != nullptr && rhs != nullptr
		       && m_vtable->equals(this, lhs, rhs);
	}

	bool Type::compare(const void* lhs, const void* rhs, i32& result) const
	{
		if (m_vtable->compare == nullptr || lhs == nullptr || rhs == nullptr)
			return false;
		result = m_vtable->compare(this, lhs, rhs);
		return true;
	}

	bool Type::serialize(void* self, Archive& ar)
	{
		if (m_vtable->serialize == nullptr || self == nullptr)
			return false;

		m_vtable->serialize(this, self, ar);
		return true;
	}

	usize Type::parse(void* self, StringView value) const
	{
		if (m_vtable->parse == nullptr || self == nullptr)
			return 0;

		return m_vtable->parse(this, self, value);
	}

	bool Type::format(const void* self, String& value) const
	{
		if (m_vtable->format == nullptr || self == nullptr)
			return false;

		return m_vtable->format(this, self, value);
	}
} // namespace Trinex::Refl
