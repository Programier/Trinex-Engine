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

    Any::Any(const Any& any) : _M_manager(any._M_manager)
    {
        if (any.has_value())
        {
            any._M_manager->copy(any._M_storage, _M_storage);
        }
    }

    Any::Any(Any&& any) : _M_manager(any._M_manager)
    {
        if (any.has_value())
        {
            any._M_manager->move(any._M_storage, _M_storage);
            any._M_manager = nullptr;
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
        return _M_manager && _M_manager->is_valid();
    }

    Any& Any::swap(Any& any)
    {
        if (this->_M_manager != any._M_manager)
        {
            Any tmp(std::move(any));
            any._M_manager = _M_manager;
            if (_M_manager != nullptr)
                _M_manager->move(_M_storage, any._M_storage);

            _M_manager = tmp._M_manager;
            if (tmp._M_manager != nullptr)
            {
                tmp._M_manager->move(tmp._M_storage, _M_storage);
                tmp._M_manager = nullptr;
            }
        }
        else
        {
            if (this->_M_manager != nullptr)
                this->_M_manager->swap(_M_storage, any._M_storage);
        }

        return *this;
    }

    Any& Any::reset()
    {
        if (has_value())
        {
            this->_M_manager->destroy(_M_storage);
            this->_M_manager = nullptr;
        }

        return *this;
    }

    Any::~Any()
    {
        reset();
    }
}// namespace Engine
