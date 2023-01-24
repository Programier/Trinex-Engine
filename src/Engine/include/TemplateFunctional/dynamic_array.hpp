#pragma once


#include <TemplateFunctional/is_container.hpp>
#include <TemplateFunctional/reference_wrapper.hpp>
#include <TemplateFunctional/smart_pointer.hpp>
#include <cstddef>
#include <iterator>

#if 0
namespace Engine
{
#define TEMPLATE template<typename ArrayType>
#define DYNAMIC_ARRAY TEMPLATE DynamicArray<ArrayType>
#define ITERATOR_TEMPLATE TEMPLATE template<typename Access>
#define INPUT_ITERATOR template<typename InputIterator>

#define make_capacity(size)                                                                                                 \
    {                                                                                                                       \
        size--;                                                                                                             \
        int tmp = 1;                                                                                                        \
        int count = sizeof(size) + 1;                                                                                       \
        while (count--)                                                                                                     \
        {                                                                                                                   \
            size |= size >> tmp;                                                                                            \
            tmp *= 2;                                                                                                       \
        }                                                                                                                   \
        size++;                                                                                                             \
    }

    template<typename ArrayType>
    class DynamicArray
    {
        using Pointer = SmartPointer<ArrayType>;

    public:
        template<typename Access>
        class basic_iterator
        {
            Pointer _M_base;
            Pointer _M_data;
            std::size_t _M_size;
            std::size_t _M_index;

        public:
#if __cplusplus >= 201703L
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = ArrayType;
            using pointer = ArrayType*;
            using reference = ArrayType&;
#endif
            basic_iterator(const Pointer& base, const Pointer& data, std::size_t size, std::size_t index);
            ArrayType* base();
            const ArrayType* base() const;

            basic_iterator& operator++();
            basic_iterator operator++(int);
            basic_iterator& operator--();
            basic_iterator operator--(int);
            bool operator==(const basic_iterator& it) const;
            bool operator!=(const basic_iterator& it) const;
            basic_iterator operator+(std::size_t value) const;
            basic_iterator& operator+=(std::size_t value);
            basic_iterator operator-(std::size_t value) const;
            basic_iterator& operator-=(std::size_t value);
            std::size_t operator-(const basic_iterator& it) const;

            bool operator<(const basic_iterator& it) const;
            bool operator<=(const basic_iterator& it) const;
            bool operator>(const basic_iterator& it) const;
            bool operator>=(const basic_iterator& it) const;
            bool is_null() const;
            std::size_t index() const;
            Access& operator*();


            operator basic_iterator<const ArrayType>();
            friend class DynamicArray;
        };

        typedef basic_iterator<ArrayType> iterator;
        typedef basic_iterator<const ArrayType> const_iterator;


    private:
        Pointer _M_base;
        Pointer _M_data;
        std::size_t _M_size = 0;
        std::size_t _M_capacity = 0;
        bool _M_need_realloc = false;

        void _M_check_size() const;

    public:
        DynamicArray();
        DynamicArray(const std::initializer_list<ArrayType>& array);
        DynamicArray(ArrayType* data, std::size_t size, bool delete_memory = false);
        DynamicArray(const DynamicArray& array, bool copy = false);
        DynamicArray(const DynamicArray&&);
        DynamicArray& operator = (DynamicArray&&);

        INPUT_ITERATOR DynamicArray(InputIterator first, InputIterator last);


        template<template<class, class...> class Container, typename... Args,
                 typename = typename std::enable_if<is_container<Container<ArrayType, Args...>>::value>::type>
        DynamicArray(const Container<ArrayType, Args...>& container);

        DynamicArray& operator=(const DynamicArray&);
        DynamicArray& copy(const DynamicArray&);

        iterator begin();
        const_iterator begin() const;
        const_iterator cbegin() const;

        iterator end();
        const_iterator end() const;
        const_iterator cend() const;


        ArrayType& front();
        const ArrayType& front() const;
        ArrayType& back();
        const ArrayType& back() const;
        DynamicArray& push_back(const ArrayType& value);
        DynamicArray& pop_back();
        DynamicArray& insert(const const_iterator& to, const ArrayType& value);

        INPUT_ITERATOR
        DynamicArray& insert(const const_iterator& to, InputIterator first, InputIterator last);

        DynamicArray& erase(const const_iterator& it);
        DynamicArray& erase(const const_iterator& first, const const_iterator& last);
        DynamicArray& clear();
        DynamicArray sub_array(const const_iterator& begin, const const_iterator& end);
        const std::size_t& size() const;
        const std::size_t& capacity() const;

        DynamicArray& resize(std::size_t new_size);
        DynamicArray& reserve(const std::size_t& new_capacity);

        bool is_empty() const;

        const ArrayType* data() const;

        ArrayType& operator[](const std::size_t& index);
        const ArrayType& operator[](const std::size_t& index) const;

        // Convert operator
        template<template<class, class...> class Container, typename... Args,
                 typename = typename std::enable_if<is_container<Container<ArrayType, Args...>>::value>::type>
        operator Container<ArrayType, Args...>();
    };


    // Implementation

    // ITERATOR IMPLEMENTATION
    ITERATOR_TEMPLATE DynamicArray<ArrayType>::basic_iterator<Access>::basic_iterator(const Pointer& base,
                                                                                      const Pointer& data,
                                                                                      std::size_t size, std::size_t index)
    {
        _M_base = base;
        _M_data = data;
        _M_size = size;
        _M_index = index;
    }

    ITERATOR_TEMPLATE ArrayType* DynamicArray<ArrayType>::basic_iterator<Access>::base()
    {
        return _M_data.get() + _M_index;
    }

    ITERATOR_TEMPLATE const ArrayType* DynamicArray<ArrayType>::basic_iterator<Access>::base() const
    {
        return _M_data.get() + _M_index;
    }


    ITERATOR_TEMPLATE typename DynamicArray<ArrayType>::template basic_iterator<Access>&
    DynamicArray<ArrayType>::basic_iterator<Access>::operator++()
    {

        _M_index = _M_index < _M_size ? _M_index + 1 : 0;
        return *this;
    }

    ITERATOR_TEMPLATE typename DynamicArray<ArrayType>::template basic_iterator<Access>
    DynamicArray<ArrayType>::basic_iterator<Access>::operator++(int)
    {
        auto result = *this;
        _M_index = _M_index < _M_size ? _M_index + 1 : 0;
        return result;
    }

    ITERATOR_TEMPLATE typename DynamicArray<ArrayType>::template basic_iterator<Access>&
    DynamicArray<ArrayType>::basic_iterator<Access>::operator--()
    {
        _M_index = _M_index > 0 ? _M_index - 1 : _M_size;
        return *this;
    }

    ITERATOR_TEMPLATE typename DynamicArray<ArrayType>::template basic_iterator<Access>
    DynamicArray<ArrayType>::basic_iterator<Access>::operator--(int)
    {
        auto result = *this;
        _M_index = _M_index > 0 ? _M_index - 1 : _M_size;
        return result;
    }

    ITERATOR_TEMPLATE std::size_t DynamicArray<ArrayType>::basic_iterator<Access>::operator-(
            const DynamicArray<ArrayType>::basic_iterator<Access>& it) const
    {
        if (_M_size == 0 || it._M_size == 0)
            return -1;
        auto e = _M_data.get() + _M_index;
        auto b = it._M_data.get() + it._M_index;
        return e < b ? b - e : e - b;
    }

    ITERATOR_TEMPLATE bool
    DynamicArray<ArrayType>::basic_iterator<Access>::operator==(const basic_iterator<Access>& it) const
    {
        if (_M_size == 0 || it._M_size == 0)
            return false;
        return _M_data.get() + _M_index == it._M_data.get() + it._M_index;
    }

    ITERATOR_TEMPLATE bool
    DynamicArray<ArrayType>::basic_iterator<Access>::operator!=(const basic_iterator<Access>& it) const
    {
        return !(*this == it);
    }

    ITERATOR_TEMPLATE typename DynamicArray<ArrayType>::template basic_iterator<Access>
    DynamicArray<ArrayType>::basic_iterator<Access>::operator+(std::size_t value) const
    {
        DynamicArray<ArrayType>::basic_iterator result = *this;
        if (_M_size != 0)
        {
            result._M_index += value;
            result._M_index %= (_M_size + 1);
        }
        return result;
    }

    ITERATOR_TEMPLATE typename DynamicArray<ArrayType>::template basic_iterator<Access>&
    DynamicArray<ArrayType>::basic_iterator<Access>::operator+=(std::size_t value)
    {
        if (_M_size != 0)
        {
            _M_index += value;
            _M_index %= (_M_size + 1);
        }
        return *this;
    }

    ITERATOR_TEMPLATE typename DynamicArray<ArrayType>::template basic_iterator<Access>
    DynamicArray<ArrayType>::basic_iterator<Access>::operator-(std::size_t value) const
    {
        DynamicArray<ArrayType>::basic_iterator<Access> result = *this;
        if (_M_size != 0)
        {
            result._M_index -= value;
            result._M_index %= (_M_size + 1);
        }
        return result;
    }

    ITERATOR_TEMPLATE typename DynamicArray<ArrayType>::template basic_iterator<Access>&
    DynamicArray<ArrayType>::basic_iterator<Access>::operator-=(std::size_t value)
    {
        if (_M_size != 0)
        {
            _M_index -= value;
            _M_index %= (_M_size + 1);
        }
        return *this;
    }

    ITERATOR_TEMPLATE bool DynamicArray<ArrayType>::basic_iterator<Access>::operator<(const basic_iterator<Access>& it) const
    {
        if (_M_size == 0 || it._M_size == 0)
            return false;
        return _M_data.get() + _M_index < it._M_data.get() + it._M_index;
    }

    ITERATOR_TEMPLATE bool
    DynamicArray<ArrayType>::basic_iterator<Access>::operator<=(const basic_iterator<Access>& it) const
    {
        if (_M_size == 0 || it._M_size == 0)
            return false;
        return _M_data.get() + _M_index <= it._M_data.get() + it._M_index;
    }

    ITERATOR_TEMPLATE bool DynamicArray<ArrayType>::basic_iterator<Access>::operator>(const basic_iterator<Access>& it) const
    {
        if (_M_size == 0 || it._M_size == 0)
            return false;
        return _M_data.get() + _M_index > it._M_data.get() + it._M_index;
    }

    ITERATOR_TEMPLATE bool
    DynamicArray<ArrayType>::basic_iterator<Access>::operator>=(const basic_iterator<Access>& it) const
    {
        if (_M_size == 0 || it._M_size == 0)
            return false;
        return _M_data.get() + _M_index >= it._M_data.get() + it._M_index;
    }

    ITERATOR_TEMPLATE bool DynamicArray<ArrayType>::basic_iterator<Access>::is_null() const
    {
        return _M_data.get() == nullptr || _M_size == 0;
    }

    ITERATOR_TEMPLATE std::size_t DynamicArray<ArrayType>::basic_iterator<Access>::index() const
    {
        return _M_index;
    }

    ITERATOR_TEMPLATE Access& DynamicArray<ArrayType>::basic_iterator<Access>::operator*()
    {
        if (_M_size == 0)
            throw std::runtime_error("nullptr exception");
        return basic_iterator::_M_data.get()[_M_index == _M_size ? _M_index - 1 : _M_index];
    }

    ITERATOR_TEMPLATE DynamicArray<ArrayType>::basic_iterator<Access>::operator basic_iterator<const ArrayType>()
    {
        return const_iterator(_M_base, _M_data, _M_size, _M_index);
    }

    //    DYNAMIC ARRAY IMPLEMENTATION

    DYNAMIC_ARRAY::DynamicArray() = default;
    DYNAMIC_ARRAY::DynamicArray(const std::initializer_list<ArrayType>& array)
    {
        _M_base = Pointer(new ArrayType[array.size()], delete_array<ArrayType>);
        _M_data = _M_base;
        _M_size = array.size();
        _M_capacity = _M_size;
        std::move(array.begin(), array.end(), _M_data.get());
    }

    TEMPLATE INPUT_ITERATOR DynamicArray<ArrayType>::DynamicArray(InputIterator first, const InputIterator last)
    {
        _M_size = _M_capacity = std::distance(first, last);
        if (_M_size == 0)
            return;
        _M_data = _M_base = Pointer(new ArrayType[_M_size], delete_array<ArrayType>);
        _M_need_realloc = false;

        std::size_t index = 0;
        while (first != last) _M_base.get()[index++] = (*first++);
    }

    TEMPLATE template<template<class, class...> class Container, typename... Args, typename _>
    DynamicArray<ArrayType>::DynamicArray(const Container<ArrayType, Args...>& container)
    {
        _M_base = Pointer(new ArrayType[container.size()], delete_array<ArrayType>);
        _M_data = _M_base;
        _M_size = container.size();
        _M_capacity = _M_size;
        std::copy(container.begin(), container.end(), _M_data.get());
    }

    DYNAMIC_ARRAY::DynamicArray(ArrayType* data, std::size_t size, bool delete_memory) : _M_size(size), _M_capacity(size)
    {
        _M_base = Pointer(data, (delete_memory ? delete_array<ArrayType> : fake_delete<ArrayType>) );
        _M_data = _M_base;
    }

    DYNAMIC_ARRAY::DynamicArray(const DynamicArray& array, bool _copy)
    {
        if (_copy)
            copy(array);
        else
            *this = array;
    }

    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::operator=(const DynamicArray<ArrayType>& array)
    {
        _M_base = array._M_base;
        _M_data = array._M_data;
        _M_capacity = array._M_capacity;
        _M_need_realloc = true;
        _M_size = array._M_size;
        return *this;
    }

    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::copy(const DynamicArray<ArrayType>& array)
    {
        _M_base = Pointer(new ArrayType[array._M_size], array._M_size);
        _M_data = _M_base;
        _M_size = array._M_size;
        _M_capacity = _M_size;
        std::copy(array._M_data.get(), array._M_data.get() + _M_size, _M_base);
        _M_need_realloc = false;
        _M_size = array._M_size;
    }

    TEMPLATE typename DynamicArray<ArrayType>::iterator DynamicArray<ArrayType>::begin()
    {
        return iterator(_M_base, _M_data, _M_size, 0);
    }

    TEMPLATE typename DynamicArray<ArrayType>::const_iterator DynamicArray<ArrayType>::begin() const
    {
        return const_iterator(_M_base, _M_data, _M_size, 0);
    }

    TEMPLATE typename DynamicArray<ArrayType>::const_iterator DynamicArray<ArrayType>::cbegin() const
    {
        return const_iterator(_M_base, _M_data, _M_size, 0);
    }

    TEMPLATE typename DynamicArray<ArrayType>::iterator DynamicArray<ArrayType>::end()
    {
        return iterator(_M_base, _M_data, _M_size, _M_size);
    }

    TEMPLATE typename DynamicArray<ArrayType>::const_iterator DynamicArray<ArrayType>::end() const
    {
        return const_iterator(_M_base, _M_data, _M_size, _M_size);
    }

    TEMPLATE typename DynamicArray<ArrayType>::const_iterator DynamicArray<ArrayType>::cend() const
    {
        return const_iterator(_M_base, _M_data, _M_size, _M_size);
    }

    TEMPLATE void DynamicArray<ArrayType>::_M_check_size() const
    {
        if (_M_size == 0)
            throw std::runtime_error("Dynamic array: Array is empty");
    }

    TEMPLATE ArrayType& DynamicArray<ArrayType>::front()
    {
        _M_check_size();
        return _M_data.get()[0];
    }

    TEMPLATE const ArrayType& DynamicArray<ArrayType>::front() const
    {
        _M_check_size();
        return _M_data.get()[0];
    }

    TEMPLATE ArrayType& DynamicArray<ArrayType>::back()
    {
        _M_check_size();
        return _M_data.get()[_M_size - 1];
    }

    TEMPLATE const ArrayType& DynamicArray<ArrayType>::back() const
    {
        _M_check_size();
        return _M_data.get()[_M_size - 1];
    }


    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::push_back(const ArrayType& value)
    {
        if (_M_size == _M_capacity || _M_need_realloc)
        {
            _M_capacity = _M_size + 1;
            make_capacity(_M_capacity);
            Pointer new_data(new ArrayType[_M_capacity], delete_array<ArrayType>);
            if (_M_size != 0)
                std::move(_M_data.get(), _M_data.get() + _M_size, new_data.get());
            _M_need_realloc = false;
            _M_data = _M_base = new_data;
        }
        _M_data.get()[_M_size++] = value;
        return *this;
    }

    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::pop_back()
    {
        if (_M_need_realloc || (_M_size - 1) * 2 <= _M_capacity)
        {
            _M_capacity = _M_size - 1;
            make_capacity(_M_capacity);
            Pointer new_data(new ArrayType[_M_capacity], delete_array<ArrayType>);
            if (_M_size != 0)
                std::move(_M_data.get(), _M_data.get() + _M_size - 1, new_data.get());
            _M_need_realloc = false;
            _M_data = _M_base = new_data;
        }
        _M_size--;
    }

    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::insert(const const_iterator& to, const ArrayType& value)
    {
        if (to.is_null() || to._M_data.get() != _M_data.get())
            return *this;

        if (_M_need_realloc || _M_size == _M_capacity)
        {
            _M_capacity = _M_size + 1;
            make_capacity(_M_capacity);
            Pointer new_data(new ArrayType[_M_capacity], delete_array<ArrayType>);
            std::move(_M_data.get(), _M_data.get() + to._M_index, new_data.get());
            std::move(_M_data.get() + to._M_index, _M_data.get() + _M_size, new_data.get() + to._M_index + 1);
            new_data.get()[to._M_index] = value;
            _M_need_realloc = false;
            _M_data = _M_base = new_data;
        }
        else
        {
            std::move(_M_data.get() + to._M_index, _M_data.get() + _M_size, _M_data.get() + to._M_index + 1);
            _M_data.get()[to._M_index] = value;
        }
        _M_size++;
        return *this;
    }

    TEMPLATE INPUT_ITERATOR DynamicArray<ArrayType>& DynamicArray<ArrayType>::insert(const const_iterator& to,
                                                                                     InputIterator first, InputIterator last)
    {
        if (to.is_null() || to._M_data.get() != _M_data.get())
            return *this;

        std::size_t diff_size = std::distance(first, last);
        if (diff_size == 0)
            return *this;

        if (_M_need_realloc || _M_size + diff_size > _M_capacity)
        {
            _M_capacity = _M_size + diff_size;
            make_capacity(_M_capacity);
            Pointer new_data(new ArrayType[_M_capacity], delete_array<ArrayType>);
            std::move(_M_data.get(), _M_data.get() + to._M_index, new_data.get());
            std::copy(first, last, new_data.get() + to._M_index);
            std::move(_M_data.get() + to._M_index, _M_data.get() + _M_size, new_data.get() + to._M_index + diff_size);
            _M_need_realloc = false;
            _M_data = _M_base = new_data;
        }
        else
        {
            std::move(_M_data.get() + to._M_index, _M_data.get() + _M_size, _M_data.get() + to._M_index + diff_size);
            std::copy(first, last, _M_data.get() + to._M_index);
        }
        _M_size += diff_size;
        return *this;
    }

    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::erase(const const_iterator& it)
    {
        if (it._M_data.get() != _M_data.get() || _M_size == 0)
            return *this;

        if (_M_need_realloc || (_M_size - 1) * 2 <= _M_capacity)
        {
            _M_capacity = _M_size - 1;
            make_capacity(_M_capacity);
            Pointer new_data(new ArrayType[_M_capacity], delete_array<ArrayType>);
            std::move(_M_data.get(), _M_data.get() + it._M_index, new_data.get());
            std::move(_M_data.get() + it._M_index + 1, _M_data.get() + _M_size, new_data.get() + it._M_index);
            _M_data = _M_base = new_data;
            _M_need_realloc = false;
        }
        else
            std::move(_M_data.get() + it._M_index + 1, _M_data.get() + _M_size, _M_data.get() + it._M_index);


        _M_size--;
        return *this;
    }

    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::erase(const const_iterator& first, const const_iterator& last)
    {
        if (first._M_data.get() != last._M_data.get() || first._M_data.get() != _M_data.get() || _M_size == 0 ||
            first >= last)
            return *this;

        std::size_t diff = last - first;

        if (_M_need_realloc || (_M_size - diff) * 2 <= _M_capacity)
        {
            _M_capacity = _M_size - diff;
            make_capacity(_M_capacity);
            Pointer new_data(new ArrayType[_M_capacity], delete_array<ArrayType>);
            std::move(_M_data.get(), _M_data.get() + first._M_index, new_data.get());
            std::move(_M_data.get() + last._M_index, _M_data.get() + _M_size, new_data.get() + first._M_index);
            _M_data = _M_base = new_data;
            _M_need_realloc = false;
        }
        else
            std::move(_M_data.get() + last._M_index, _M_data.get() + _M_size, _M_data.get() + first._M_index);


        _M_size -= diff;
        return *this;
    }

    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::clear()
    {
        _M_size = 0;
        _M_capacity = 0;
        _M_need_realloc = false;
        _M_base.reset();
        _M_data.reset();
        return *this;
    }

    TEMPLATE DynamicArray<ArrayType> DynamicArray<ArrayType>::sub_array(const const_iterator& _begin,
                                                                        const const_iterator& _end)
    {
        if (_begin > _end || _begin.is_null() || _end.is_null())
            throw std::runtime_error("Dynamic array: Assertion _begin <= _end failed");
        DynamicArray<ArrayType> result;
        result._M_base = _M_base;
        result._M_size = _end - _begin;
        result._M_capacity = result._M_size;
        result._M_need_realloc = true;
        result._M_data = Pointer(_begin._M_data.get() + _begin._M_index, fake_delete<ArrayType>);
        return result;
    }

    TEMPLATE const std::size_t& DynamicArray<ArrayType>::size() const
    {
        return _M_size;
    }

    TEMPLATE const std::size_t& DynamicArray<ArrayType>::capacity() const
    {
        return _M_capacity;
    }

    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::resize(std::size_t new_size)
    {
        if (new_size == 0)
            return clear();

        auto _capacity = new_size;
        make_capacity(_capacity);
        Pointer new_data(new ArrayType[_capacity], delete_array<ArrayType>);
        //std::copy(this->, new_size > _M_size ? this->end() : this->begin() + new_size, iterator(new_data, new_data, new_size, 0));
        std::move(_M_data.get(), _M_data.get() + std::min(new_size, _M_size), new_data.get());
        _M_size = new_size;
        _M_capacity = _capacity;
        _M_base = _M_data = new_data;
        _M_need_realloc = false;
        return *this;
    }

    TEMPLATE DynamicArray<ArrayType>& DynamicArray<ArrayType>::reserve(const std::size_t& new_capacity)
    {
        if (new_capacity < _M_size || !_M_capacity)
            return *this;

        Pointer ptr(new ArrayType[new_capacity], delete_array<ArrayType>);
        std::move(begin(), end(), ptr.get());
        _M_capacity = new_capacity;
        _M_data = _M_base = ptr;
    }

    TEMPLATE bool DynamicArray<ArrayType>::is_empty() const
    {
        return _M_size == 0;
    }

    TEMPLATE const ArrayType* DynamicArray<ArrayType>::data() const
    {
        return _M_data.get();
    }


    TEMPLATE ArrayType& DynamicArray<ArrayType>::operator[](const std::size_t& index)
    {
        _M_check_size();
        return _M_data.get()[index % (_M_size + 1)];
    }

    TEMPLATE const ArrayType& DynamicArray<ArrayType>::operator[](const std::size_t& index) const
    {
        _M_check_size();
        return _M_data.get()[index % (_M_size + 1)];
    }

    TEMPLATE
    DynamicArray<ArrayType>::DynamicArray(const DynamicArray&&)
    {

    }

    TEMPLATE
    DynamicArray<ArrayType>& DynamicArray<ArrayType>::operator = (DynamicArray&& array)
    {
        if(this == &array)
            return *this;

        _M_base = std::move(array._M_base);
        _M_data = std::move(array._M_data);
        _M_size = array._M_size;
        array._M_size = 0;
        _M_capacity = array._M_capacity;
        _M_need_realloc = array._M_need_realloc;
        array._M_capacity = 0;
        array._M_need_realloc = false;
        return *this;
    }

    TEMPLATE template<template<class, class...> class Container, typename... Args, typename _>
    DynamicArray<ArrayType>::operator Container<ArrayType, Args...>()
    {
        return Container<ArrayType, Args...>(begin(), end());
    }


#undef DYNAMIC_ARRAY
#undef TEMPLATE
#undef make_capacity
}// namespace Engine
#endif
