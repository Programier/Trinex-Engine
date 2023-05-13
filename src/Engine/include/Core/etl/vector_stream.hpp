#pragma once
#include <algorithm>
#include <cstring>
#include <istream>
#include <ostream>
#include <streambuf>

#include <Core/engine_types.hpp>

namespace Engine
{
    template<typename ElementType>
    class BasicVectorStreamBuff : public std::basic_streambuf<ElementType>
    {
    public:
        using size_type = typename Vector<ElementType>::size_type;
        using pos_type  = typename std::basic_streambuf<ElementType>::pos_type;
        using off_type  = typename std::basic_streambuf<ElementType>::off_type;
        using int_type  = typename std::basic_streambuf<ElementType>::int_type;

    private:
        Vector<ElementType>& _M_buffer;
        size_type _M_read_pos;
        size_type _M_write_pos;

    public:
        const Vector<ElementType>& vector() const
        {
            return _M_buffer;
        }

        BasicVectorStreamBuff(Vector<ElementType>& buffer) : _M_buffer(buffer), _M_read_pos(0), _M_write_pos(0)
        {}

        pos_type seekpos(pos_type pos, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        {
            return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
        }

        pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                         std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        {

            size_type& pos = (mode & std::ios_base::out ? _M_write_pos : _M_read_pos);

            switch (dir)
            {
                case std::ios_base::beg:
                    pos = off;
                    break;
                case std::ios_base::cur:
                    pos += off;
                    break;
                case std::ios_base::end:
                    pos = _M_buffer.size() + off;
                    break;
                default:
                    break;
            }

            pos = std::min(_M_buffer.size(), pos);
            return pos_type(pos);
        }

        int_type overflow(int_type c)
        {
            if (c != static_cast<int_type>(EOF))
            {
                typename Vector<ElementType>::size_type current_size = _M_buffer.size();

                if (_M_write_pos < current_size)
                {
                    _M_buffer[_M_write_pos++] = static_cast<ElementType>(c);
                }
                else if (_M_write_pos == current_size)
                {
                    _M_buffer.push_back(static_cast<ElementType>(c));
                    ++_M_write_pos;
                }
            }
            return c;
        }

        std::streamsize xsputn(const ElementType* s, std::streamsize num)
        {

            if (_M_write_pos + static_cast<size_type>(num) < _M_buffer.size())
            {
                std::copy_n(s, num, _M_buffer.begin() + _M_write_pos);
            }
            else
            {
                size_type write_elements = _M_buffer.size() - _M_write_pos;
                if (write_elements != 0)
                    std::copy_n(s, write_elements, _M_buffer.begin() + _M_write_pos);
                _M_buffer.insert(_M_buffer.end(), s + write_elements, s + num - write_elements);
            }
            _M_write_pos += static_cast<size_type>(num);
            return num;
        }

        int_type underflow()
        {
            if (_M_read_pos < _M_buffer.size())
                return static_cast<int_type>(_M_buffer[_M_read_pos]);
            else
                return EOF;
        }

        int_type uflow()
        {
            if (_M_read_pos < _M_buffer.size())
                return static_cast<int_type>(_M_buffer[_M_read_pos++]);
            else
                return static_cast<int_type>(EOF);
        }

        int_type pbackfail(int_type c)
        {
            const auto prev = _M_read_pos - 1;
            if (c != static_cast<int_type>(EOF) && prev < _M_buffer.size() &&
                c != static_cast<int_type>(_M_buffer[prev]))
            {
                return static_cast<int_type>(EOF);
            }

            _M_read_pos = prev;
            return 1;
        }

        std::streamsize xsgetn(ElementType* s, std::streamsize n)
        {
            if (_M_read_pos < _M_buffer.size())
            {
                const size_type num = std::min<size_type>(n, _M_buffer.size() - _M_read_pos);
                std::memcpy(s, &_M_buffer[_M_read_pos], num);
                _M_read_pos += num;
                return num;
            }
            return 0;
        }
    };

    using VectorStreamBuff = BasicVectorStreamBuff<signed_byte>;

    template<typename ElementType>
    class BasicVectorOutputStream : public std::basic_ostream<ElementType>
    {
    private:
        Vector<ElementType> _M_vector;
        BasicVectorStreamBuff<ElementType> _M_buffer;

    public:
        using Stream = std::basic_ostream<ElementType>;
        BasicVectorOutputStream() : std::basic_ostream<ElementType>(&_M_buffer), _M_buffer(_M_vector)
        {}

        const Vector<ElementType>& vector() const
        {
            return _M_vector;
        }
    };

    template<typename ElementType>
    class BasicVectorInputStream : public std::basic_istream<ElementType>
    {
    private:
        BasicVectorStreamBuff<ElementType> _M_buffer;

    public:
        using Stream = std::basic_istream<ElementType>;
        BasicVectorInputStream(Vector<ElementType>& vector)
            : std::basic_istream<ElementType>(&_M_buffer), _M_buffer(vector)
        {}

        const Vector<ElementType>& vector() const
        {
            return _M_buffer.vector();
        }
    };

    using VectorOutputStream = BasicVectorOutputStream<char>;
    using VectorInputStream  = BasicVectorInputStream<char>;

}// namespace Engine
