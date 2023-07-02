#pragma once
#include <Core/engine_types.hpp>
#include <Core/exception.hpp>

namespace Engine
{
    template<typename Type>
    class RingBuffer
    {
    private:
        Vector<Type> _M_data;
        Type* _M_read_ptr  = nullptr;
        Type* _M_write_ptr = nullptr;


        FORCE_INLINE void check_size()
        {
            if(size() == _M_data.size())
            {
                throw EngineException("Ring buffer overflow");
            }
        }

    public:
        RingBuffer(size_t size) : _M_data(std::max(static_cast<size_t>(1), size))
        {
            _M_read_ptr  = _M_data.data();
            _M_write_ptr = _M_data.data();
        }

        RingBuffer& push(const Type& value)
        {
            check_size();
            *_M_write_ptr = value;
            _M_write_ptr++;

            if (_M_write_ptr == _M_data.data() + _M_data.size())
                _M_write_ptr = _M_data.data();
            return *this;
        }

        RingBuffer& push(Type&& value)
        {
            check_size();
            (*_M_write_ptr) = std::move(value);
            _M_write_ptr++;

            if (_M_write_ptr == _M_data.data() + _M_data.size())
                _M_write_ptr = _M_data.data();

            return *this;
        }

        RingBuffer& pop()
        {
            if (empty())
                return *this;

            _M_read_ptr++;

            if (_M_read_ptr == _M_data.data() + _M_data.size())
                _M_read_ptr = _M_data.data();
            return *this;
        }

        Type& front()
        {
            return *_M_read_ptr;
        }

        const Type& front() const
        {
            return *_M_read_ptr;
        }

        bool empty() const
        {
            return _M_read_ptr == _M_write_ptr;
        }

        size_t size() const
        {
            return static_cast<size_t>(_M_write_ptr - _M_read_ptr + (_M_write_ptr < _M_read_ptr ? _M_data.size() : 0));
        }

        RingBuffer& clear()
        {
            _M_read_ptr = _M_write_ptr = _M_data.data();
            return *this;
        }

        size_t max_size() const
        {
            return _M_data.size();
        }
    };
}
