#pragma once

#include <Core/etl/constexpr_string.hpp>
#include <Core/exception.hpp>
#include <Core/string_functions.hpp>

class CScriptArray;
class asIScriptEngine;
class asITypeInfo;

namespace Engine
{
#define script_array_init_check(fail_ret)                                                                                        \
    if (m_as_array == nullptr)                                                                                                   \
        return fail_ret;

#define script_array_init_check_noret(fail_ret)                                                                                  \
    if (m_as_array == nullptr)                                                                                                   \
    {                                                                                                                            \
        fail_ret;                                                                                                                \
    }

    class ENGINE_EXPORT ScriptArrayBase
    {
    public:
        using size_type       = size_t;
        using difference_type = ptrdiff_t;

    protected:
        CScriptArray* m_as_array;

        virtual const String& full_declaration() = 0;

    protected:
        int find_object_type_id();
        asITypeInfo* find_object_type();

        void insert_last(const void* ptr);
        void* element_at(size_type pos) const;

        void do_copy(const ScriptArrayBase* from);
        void do_move(ScriptArrayBase* from);

    public:
        ScriptArrayBase();

        bool create(size_type init_size = 0);
        bool attach(CScriptArray* array, bool add_reference = true);
        ScriptArrayBase& release();
        CScriptArray* ref(bool inc_ref_count = false);
        size_type size() const;
        ScriptArrayBase& resize(size_type n);
        bool empty() const;
        ScriptArrayBase& reserve(size_type n);


        virtual ~ScriptArrayBase();
    };

    template<class T, ConstexprString declaration>
    class ScriptArray : public ScriptArrayBase
    {
    public:
        using value_type      = T;
        using pointer         = T*;
        using const_pointer   = const T*;
        using reference       = T&;
        using const_reference = const T&;


    protected:
        const String& full_declaration() override
        {
            static String result = Strings::format("array<{}>", declaration.c_str());
            return result;
        }

    public:
        template<typename Type>
        class Iterator
        {
        private:
            ScriptArray* m_script_array;
            size_type m_position;

        public:
            using value_type      = Type;
            using pointer         = Type*;
            using const_pointer   = const Type*;
            using reference       = Type&;
            using const_reference = const Type&;

            Iterator(ScriptArray* array, size_type position) : m_script_array(array), m_position(position)
            {}

            Iterator(const Iterator& other) : Iterator(other.m_script_array, other.m_position)
            {}

            reference operator*() const
            {
                return (*m_script_array)[m_position];
            }

            Iterator& operator++()
            {
                ++m_position;
                return *this;
            }

            Iterator operator++(int)
            {
                Iterator copy(*this);
                ++m_position;
                return copy;
            }

            Iterator& operator--()
            {
                --m_position;
                return *this;
            }

            Iterator operator--(int)
            {
                Iterator copy(*this);
                --m_position;
                return copy;
            }

            bool operator==(const Iterator& other) const
            {
                return (m_script_array == other.m_script_array) && (m_position == other.m_position);
            }

            bool operator!=(const Iterator& other) const
            {
                return (m_script_array != other.m_script_array) || (m_position == other.m_position);
            }

            bool operator<(const Iterator& other) const
            {
                return m_position < other.m_position;
            }

            bool operator>(const Iterator& other) const
            {
                return m_position > other.m_position;
            }

            bool operator<=(const Iterator& other) const
            {
                return m_position <= other.m_position;
            }

            bool operator>=(const Iterator& other) const
            {
                return m_position >= other.m_position;
            }

            Iterator& operator+=(size_type offset)
            {
                m_position += offset;
                return *this;
            }

            Iterator& operator-=(size_type offset)
            {
                m_position -= offset;
                return *this;
            }

            Iterator operator+(size_type offset) const
            {
                return Iterator(m_script_array, m_position + offset);
            }

            Iterator operator-(size_type offset) const
            {
                return Iterator(m_script_array, m_position + offset);
            }

            size_type operator-(const Iterator& other) const
            {
                return m_position - other.m_position;
            }

            friend Iterator operator+(size_type offset, const Iterator& it)
            {
                return Iterator(it.m_script_array, it.m_position + offset);
            }
        };

        using iterator               = Iterator<value_type>;
        using const_iterator         = Iterator<const value_type>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        ScriptArray() = default;

        ScriptArray(const ScriptArray& other)
        {
            do_copy(&other);
        }

        ScriptArray(ScriptArray&& other)
        {
            do_move(&other);
        }

        ScriptArray& operator=(const ScriptArray& other)
        {
            if (this == &other)
                return *this;
            do_copy(&other);
            return *this;
        }

        ScriptArray& operator=(ScriptArray& other)
        {
            if (this == &other)
                return *this;
            do_move(&other);
            return *this;
        }

        using ScriptArrayBase::create;

        iterator begin()
        {
            return iterator(this, 0);
        }

        iterator end()
        {
            return iterator(this, size());
        }

        const_iterator cbegin() const
        {
            return const_iterator(this, 0);
        }

        const_iterator cend() const
        {
            return const_iterator(this, size());
        }

        iterator begin() const
        {
            return cbegin();
        }

        iterator end() const
        {
            return cend();
        }

        reverse_iterator rbegin()
        {
            return reverse_iterator(end());
        }

        reverse_iterator rend()
        {
            return reverse_iterator(begin());
        }

        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator(cend());
        }

        const_reverse_iterator crend() const
        {
            return const_reverse_iterator(cbegin());
        }

        const_reverse_iterator rbegin() const
        {
            return crbegin();
        }

        const_reverse_iterator rend() const
        {
            return crend();
        }

        reference operator[](size_type index)
        {
            return at(index);
        }

        const_reference operator[](size_type index) const
        {
            return at(index);
        }

        reference at(size_type index)
        {
            return (*reinterpret_cast<pointer>(element_at(index)));
        }

        const_reference at(size_type index) const
        {
            return (*reinterpret_cast<const_pointer>(element_at(index)));
        }

        reference front()
        {
            return (*this)[0];
        }

        const_reference front() const
        {
            return (*this)[0];
        }

        reference back()
        {
            return (*this)[size() - 1];
        }

        const_reference back() const
        {
            return (*this)[size() - 1];
        }

        ScriptArray& push_back(const value_type& value)
        {
            insert_last(&value);
            return *this;
        }

        ScriptArray& pop_back()
        {
            resize(size() - 1);
            return *this;
        }

        template<class InputIterator>
        void assign(InputIterator first, InputIterator last)
        {
            resize(0);
            for (auto it = first; it < last; it++)
            {
                push_back(*it);
            }
        }

        void assign(size_type n, const value_type& val)
        {
            resize(n);

            for (size_type i = 0; i < n; ++i)
            {
                (*this)[i] = val;
            }
        }

        void clear()
        {
            resize(0);
        }
    };

#undef script_array_init_check
#undef script_array_init_check_noret

}// namespace Engine
