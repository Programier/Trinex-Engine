#include <Core/etl/any.hpp>

namespace Engine
{
    AnyBase::AnyBase() = default;
    AnyBase::AnyBase(const AnyBase& obj)
    {
        *this = obj;
    }
    AnyBase::AnyBase(AnyBase&&)            = default;
    AnyBase& AnyBase::operator=(AnyBase&&) = default;

    AnyBase& AnyBase::operator=(const AnyBase& obj)
    {
        if (this != &obj)
        {
            resize(obj.size());
            _M_info             = obj._M_info;
            _M_copy_constructor = obj._M_copy_constructor;
            _M_destructor       = obj._M_destructor;

            if (_M_copy_constructor)
                _M_copy_constructor(obj.data(), _M_buffer.data());
            else
            {
                std::memcpy(_M_buffer.data(), obj._M_buffer.data(), obj.size());
            }
        }

        return *this;
    }


    AnyBase& AnyBase::resize(size_t new_size)
    {
        clear();
        _M_buffer.resize(new_size);
        return *this;
    }

    AnyBase& AnyBase::clear()
    {
        if (!_M_buffer.empty())
        {
            if (_M_destructor)
            {
                _M_destructor(_M_buffer.data());
                _M_destructor = nullptr;
            }

            _M_buffer.clear();
            _M_buffer.shrink_to_fit();
        }

        _M_copy_constructor = nullptr;
        return *this;
    }

    size_t AnyBase::size() const
    {
        return _M_buffer.size();
    }

    byte* AnyBase::data()
    {
        return _M_buffer.data();
    }

    const byte* AnyBase::data() const
    {
        return _M_buffer.data();
    }

    bool AnyBase::empty() const
    {
        return _M_buffer.empty();
    }

    AnyBase::~AnyBase()
    {
        clear();
    }

}// namespace Engine
