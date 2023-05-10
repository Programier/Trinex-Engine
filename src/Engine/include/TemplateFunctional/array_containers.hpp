#pragma once
#include <Core/constants.hpp>
#include <Core/implement.hpp>
#include <algorithm>
#include <stdexcept>



namespace Engine
{
#define Base Vector<Type>

#define declare_container_functions(name, base)                                                                                                           \
    name() : base<Type>()                                                                                                                                 \
    {}                                                                                                                                                    \
    name(const name&) = default;                                                                                                                          \
    name(name&&) = default;                                                                                                                               \
    name& operator=(const name&) = default;                                                                                                               \
    name& operator=(name&&) = default;                                                                                                                    \
    name(ArrayIndex size) : base<Type>(size)                                                                                                              \
    {}                                                                                                                                                    \
    name(ArrayIndex size, const Type& value) : base<Type>(size, value)                                                                                    \
    {}                                                                                                                                                    \
                                                                                                                                                          \
    const Type* data() const                                                                                                                              \
    {                                                                                                                                                     \
        return Base::data();                                                                                                                              \
    }                                                                                                                                                     \
    name& reserve(ArrayIndex size)                                                                                                                        \
    {                                                                                                                                                     \
        Base::reserve(size);                                                                                                                              \
        return *this;                                                                                                                                     \
    }                                                                                                                                                     \
                                                                                                                                                          \
    Type& operator[](ArrayIndex index)                                                                                                                    \
    {                                                                                                                                                     \
        if (index >= this->size())                                                                                                                        \
            throw std::runtime_error("Index out of range!");                                                                                              \
        return Base::operator[](index);                                                                                                                   \
    }                                                                                                                                                     \
                                                                                                                                                          \
    const Type& operator[](ArrayIndex index) const                                                                                                        \
    {                                                                                                                                                     \
        if (index >= this->size())                                                                                                                        \
            throw std::runtime_error("Index out of range!");                                                                                              \
        return Base::operator[](index);                                                                                                                   \
    }                                                                                                                                                     \
                                                                                                                                                          \
    auto begin()                                                                                                                                          \
    {                                                                                                                                                     \
        return Base::begin();                                                                                                                             \
    }                                                                                                                                                     \
                                                                                                                                                          \
    auto begin() const                                                                                                                                    \
    {                                                                                                                                                     \
        return Base::begin();                                                                                                                             \
    }                                                                                                                                                     \
                                                                                                                                                          \
    auto end()                                                                                                                                            \
    {                                                                                                                                                     \
        return Base::cend();                                                                                                                              \
    }                                                                                                                                                     \
                                                                                                                                                          \
    auto end() const                                                                                                                                      \
    {                                                                                                                                                     \
        return Base::cend();                                                                                                                              \
    }                                                                                                                                                     \
                                                                                                                                                          \
    auto cbegin()                                                                                                                                         \
    {                                                                                                                                                     \
        return Base::cbegin();                                                                                                                            \
    }                                                                                                                                                     \
                                                                                                                                                          \
    auto cbegin() const                                                                                                                                   \
    {                                                                                                                                                     \
        return Base::cbegin();                                                                                                                            \
    }                                                                                                                                                     \
                                                                                                                                                          \
    auto cend()                                                                                                                                           \
    {                                                                                                                                                     \
        return Base::cend();                                                                                                                              \
    }                                                                                                                                                     \
                                                                                                                                                          \
    auto cend() const                                                                                                                                     \
    {                                                                                                                                                     \
        return Base::cend();                                                                                                                              \
    }                                                                                                                                                     \
                                                                                                                                                          \
    bool empty() const                                                                                                                                    \
    {                                                                                                                                                     \
        return Base::empty();                                                                                                                             \
    }                                                                                                                                                     \
                                                                                                                                                          \
    void remove(const Type& value)                                                                                                                        \
    {                                                                                                                                                     \
        ArrayIndex _M_index = element_index(value);                                                                                                       \
        if (_M_index != Constants::index_none)                                                                                                            \
            Base::erase(Base::begin() + _M_index);                                                                                                        \
    }                                                                                                                                                     \
                                                                                                                                                          \
    bool is_valid_index(ArrayIndex index) const                                                                                                           \
    {                                                                                                                                                     \
        return index < Base::size();                                                                                                                      \
    }                                                                                                                                                     \
                                                                                                                                                          \
    void remove_by_index(ArrayIndex index)                                                                                                                \
    {                                                                                                                                                     \
        if (is_valid_index(index))                                                                                                                        \
            Base::erase(Base::begin() + index);                                                                                                           \
    }                                                                                                                                                     \
                                                                                                                                                          \
    Type& back()                                                                                                                                          \
    {                                                                                                                                                     \
        return Base::back();                                                                                                                              \
    }                                                                                                                                                     \
                                                                                                                                                          \
    const Type& back() const                                                                                                                              \
    {                                                                                                                                                     \
        return Base::back();                                                                                                                              \
    }                                                                                                                                                     \
                                                                                                                                                          \
    Type& front()                                                                                                                                         \
    {                                                                                                                                                     \
        return Base::front();                                                                                                                             \
    }                                                                                                                                                     \
                                                                                                                                                          \
    const Type& front() const                                                                                                                             \
    {                                                                                                                                                     \
        return Base::front();                                                                                                                             \
    }

    enum class OnNoneIndex : int
    {
        None = 0,
        UseRight = 1,
        UseLeft = 2,
        Current = 3,
    };

    template<typename Type>
    class DynamicArray : protected Vector<Type>
    {
    public:
        declare_container_functions(DynamicArray, Vector);

        DynamicArray(const std::initializer_list<Type>& list) : Base(list)
        {}

        bool contains(const Type& value) const
        {
            return std::find(this->begin(), this->end(), value) != this->end();
        }

        ArrayIndex element_index(const Type& value) const
        {
            auto it = std::find(this->begin(), this->end(), value);
            if (it == this->end())
                return Constants::index_none;
            return it - this->begin();
        }

        DynamicArray& sort()
        {
            std::sort(this->begin(), this->end());
            return *this;
        }

        DynamicArray& resize(ArrayIndex size)
        {
            Base::resize(size);
            return *this;
        }
    };

    template<typename Type>
    struct BasicComparator {
        // if a > b -> return 1, if a == b return 0, if a < b return -1
        int operator()(const Type& a, const Type& b) const
        {
            if (a == b)
                return 0;
            if (a < b)
                return -1;
            return 1;
        }
    };


    template<typename Type, typename Comparator = BasicComparator<Type>, bool is_set = false>
    class SortedDynamicArray : protected Vector<Type>
    {
    private:
        Comparator comparator;

    public:
        declare_container_functions(SortedDynamicArray, Vector);


        std::size_t size() const
        {
            return Base::size();
        }

        SortedDynamicArray(const std::initializer_list<Type>& list)
        {
            reserve(list.size());
            for (auto& ell : list) push(ell);
        }

        template<typename Type2, typename Compare>
        ArrayIndex element_index(const Type2& value, Compare comp, OnNoneIndex on_none_index = OnNoneIndex::None) const
        {
            if (this->empty())
                return Constants::index_none;

            ArrayIndex left = this->size() - 1;
            ArrayIndex right = 0;

            while (right < left)
            {
                auto center = (left + right) / 2;
                auto& current_value = Base::operator[](center);
                int compare = comp(current_value, value);
                if (compare == 1)
                {
                    left = center;
                }
                else if (compare == 0)
                {
                    return center;
                }
                else
                {
                    right = center + 1;
                }
            }

            int compared = comp(Base::operator[](right), value);

            if (compared == 0 || on_none_index == OnNoneIndex::Current)
                return right;

            if (on_none_index == OnNoneIndex::UseRight)
                right = compared == -1 ? right : right - 1;
            else if (on_none_index == OnNoneIndex::UseLeft)
                right = compared == 1 ? right : right + 1;

            return right < Base::size() ? right : Constants::index_none;
        }

        ArrayIndex element_index(const Type& value, OnNoneIndex on_none_index = OnNoneIndex::None) const
        {
            return element_index(value, comparator, on_none_index);
        }

        SortedDynamicArray& push(const Type& value)
        {
            if (is_set && contains(value))
                return *this;

            ArrayIndex _M_index = this->element_index(value, OnNoneIndex::Current);
            if (_M_index == Constants::index_none)
            {
                _M_index = 0;
            }

            if (_M_index == this->size() - 1 && Base::operator[](_M_index) < value)
                ++_M_index;

            Base::insert(this->begin() + _M_index, value);
            return *this;
        }

        bool contains(const Type& value) const
        {
            return element_index(value) != Constants::index_none;
        }

        SortedDynamicArray& sort()
        {
            static auto predicate = [&](const Type& a, const Type& b) { return comparator(a, b) != 1; };

            std::sort(Base::begin(), Base::end(), predicate);
            return *this;
        }
    };


    template<typename Type, typename comparator = BasicComparator<Type>>
    using SortedDynamicSet = SortedDynamicArray<Type, comparator, true>;

}// namespace Engine
