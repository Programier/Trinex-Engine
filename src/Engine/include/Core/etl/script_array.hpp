#pragma once

#include <Core/etl/constexpr_string.hpp>
#include <Core/exception.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>
#include <scriptarray.h>

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

    template<class T, ConstexprString declaration>
    class ScriptArray
    {
    public:
        using value_type      = T;
        using pointer         = T*;
        using const_pointer   = const T*;
        using reference       = T&;
        using const_reference = const T&;
        using size_type       = size_t;
        using difference_type = ptrdiff_t;


    private:
        static asIScriptEngine* engine()
        {
            ScriptEngine* script_engine = ScriptEngine::instance();
            if (script_engine)
            {
                return script_engine->as_engine();
            }

            return nullptr;
        }

        static const String& full_declaration()
        {
            static String result = Strings::format("array<{}>", declaration.c_str());
            return result;
        }

        static int find_object_type_id()
        {
            return engine()->GetTypeIdByDecl(full_declaration().c_str());
        }

        static asITypeInfo* find_object_type()
        {
            auto id = find_object_type_id();
            return engine()->GetTypeInfoById(id);
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

        ScriptArray() : m_as_array(NULL)
        {}

        ~ScriptArray()
        {
            release();
        }

        bool create(size_type init_size = 0)
        {
            asITypeInfo* info = find_object_type();
            if (info == nullptr)
                return false;
            CScriptArray* array = CScriptArray::Create(info, init_size);
            bool result         = attach(array, false);
            if (!result)
            {
                array->Release();
            }

            return result;
        }

        bool attach(CScriptArray* array, bool add_reference = true)
        {
            if (array->GetArrayTypeId() != find_object_type_id())
                return false;

            release();

            if (add_reference)
            {
                array->AddRef();
            }

            m_as_array = array;
            return true;
        }

        void release()
        {
            if (m_as_array && engine())
            {
                m_as_array->Release();
                m_as_array = nullptr;
            }
        }

        CScriptArray* ref(bool inc_ref_count = false)
        {
            if (m_as_array == nullptr)
                return nullptr;

            if (inc_ref_count)
            {
                m_as_array->AddRef();
            }

            return m_as_array;
        }

        size_type size() const
        {
            if (m_as_array == nullptr)
                return 0;
            return static_cast<size_type>(m_as_array->GetSize());
        }

        void resize(size_type n)
        {
            script_array_init_check();
            m_as_array->Resize(n);
        }

        bool empty() const
        {
            script_array_init_check(true);
            return m_as_array->IsEmpty();
        }

        void reserve(size_type n)
        {
            script_array_init_check();
            m_as_array->Reserve(n);
        }

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
            script_array_init_check_noret(throw EngineException("Array is not initialized!"));
            pointer element = reinterpret_cast<pointer>(m_as_array->At(index));
            return (*element);
        }

        const_reference operator[](size_type index) const
        {
            script_array_init_check_noret(throw EngineException("Array is not initialized!"));
            pointer element = reinterpret_cast<pointer>(m_as_array->At(index));
            return (*element);
        }

        reference at(size_type index)
        {

            assert((m_as_array != NULL) && "InitArray() must be called before use.");

            pointer element = (pointer) m_as_array->At(index);
            if (element == NULL)
            {
                throw std::out_of_range("pos out of range");
            }
            return (*element);
        }

        const_reference at(size_type index) const
        {
            script_array_init_check_noret(throw EngineException("Array is not initialized!"));
            const_pointer element = reinterpret_cast<const_pointer>(m_as_array->At(index));

            if (element == nullptr)
            {
                throw EngineException("Index out of range");
            }
            return (*element);
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

        void push_back(const value_type& val)
        {
            script_array_init_check();
            m_as_array->InsertLast((void*) &val);
        }

        void pop_back()
        {
            resize(size() - 1);
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

    private:
        CScriptArray* m_as_array;
    };

#undef script_array_init_check
#undef script_array_init_check_noret

}// namespace Engine
