#include <Core/etl/any.hpp>
#include <Core/exception.hpp>
#include <cstring>

namespace Engine
{
	const char* Any::bad_any_cast::what() const noexcept
	{
		return "bad any cast";
	}

	Any::Storage::Storage()
	{
		memset(&stack, 0, sizeof(stack));
	}

	Any::Manager::Manager()
	{
		reset();
	}

	void Any::Manager::reset()
	{
		destroy = nullptr;
		copy    = nullptr;
		move    = nullptr;
		swap    = nullptr;
	}

	bool Any::Manager::is_valid() const
	{
		return destroy && copy && move && swap;
	}

	Any::Any() = default;

	Any::Any(const Any& any) : m_manager(any.m_manager)
	{
		if (any.has_value())
		{
			any.m_manager->copy(any.m_storage, m_storage);
		}
	}

	Any::Any(Any&& any) : m_manager(any.m_manager)
	{
		if (any.has_value())
		{
			any.m_manager->move(any.m_storage, m_storage);
			any.m_manager = nullptr;
		}
	}

	Any& Any::operator=(const Any& any)
	{
		Any(any).swap(*this);
		return *this;
	}

	Any& Any::operator=(Any&& any)
	{
		Any(std::move(any)).swap(*this);
		return *this;
	}

	bool Any::has_value() const
	{
		return m_manager && m_manager->is_valid();
	}

	Any& Any::swap(Any& any)
	{
		if (this->m_manager != any.m_manager)
		{
			Any tmp(std::move(any));
			any.m_manager = m_manager;
			if (m_manager != nullptr)
				m_manager->move(m_storage, any.m_storage);

			m_manager = tmp.m_manager;
			if (tmp.m_manager != nullptr)
			{
				tmp.m_manager->move(tmp.m_storage, m_storage);
				tmp.m_manager = nullptr;
			}
		}
		else
		{
			if (this->m_manager != nullptr)
				this->m_manager->swap(m_storage, any.m_storage);
		}

		return *this;
	}

	Any& Any::reset()
	{
		if (has_value())
		{
			this->m_manager->destroy(m_storage);
			this->m_manager = nullptr;
		}

		return *this;
	}

	Any::~Any()
	{
		reset();
	}
}// namespace Engine
