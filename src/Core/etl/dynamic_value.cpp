#include <Core/etl/dynamic_value.hpp>
#include <Core/reflection/new/const_type.hpp>
#include <Core/reflection/new/type.hpp>

namespace Trinex
{
	static Refl::New::Type* remove_const(Refl::New::Type* type)
	{
		if (type == nullptr)
			return nullptr;

		if (auto const_type = type->cast<Refl::New::ConstType>())
		{
			return const_type->type();
		}

		return type;
	}

	DynamicValue::DynamicValue(Refl::New::Type* type)
	{
		construct(type);
	}

	DynamicValue::DynamicValue(const DynamicValue& other)
	{
		if (other.m_type != nullptr && other.m_type->copy_construct(data(), other.data()))
			m_type = other.m_type;
	}

	DynamicValue::DynamicValue(DynamicValue&& other)
	{
		if (other.m_type != nullptr && other.m_type->move_construct(data(), other.data()))
		{
			m_type = other.m_type;
			other.reset();
		}
	}

	DynamicValue::~DynamicValue()
	{
		reset();
	}

	DynamicValue& DynamicValue::operator=(const DynamicValue& other)
	{
		if (this != &other)
		{
			reset();

			if (other.m_type != nullptr && other.m_type->copy_construct(data(), other.data()))
				m_type = other.m_type;
		}

		return *this;
	}

	DynamicValue& DynamicValue::operator=(DynamicValue&& other)
	{
		if (this != &other)
		{
			reset();

			if (other.m_type != nullptr && other.m_type->move_construct(data(), other.data()))
			{
				m_type = other.m_type;
				other.reset();
			}
		}

		return *this;
	}

	bool DynamicValue::construct(Refl::New::Type* type)
	{
		reset();

		type = remove_const(type);

		if (type == nullptr || type->size() > sizeof(m_storage) || type->alignment() > alignof(Storage<32, 16>) ||
		    !type->construct(data()))
		{
			return false;
		}

		m_type = type;
		return true;
	}

	bool DynamicValue::assign(Refl::New::Type* type, const void* value)
	{
		reset();

		type = remove_const(type);

		if (type == nullptr || value == nullptr || type->size() > sizeof(m_storage) ||
		    type->alignment() > alignof(Storage<32, 16>) || !type->copy_construct(data(), value))
		{
			return false;
		}

		m_type = type;
		return true;
	}

	void DynamicValue::reset()
	{
		if (m_type != nullptr)
		{
			m_type->destruct(data());
			m_type = nullptr;
		}
	}
}// namespace Trinex
