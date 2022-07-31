#pragma once

#include <BasicFunctional/smart_pointer.hpp>
#include <memory>

namespace Engine
{
    template<typename Type>
    class ReferenceWrapper
    {
    private:
        SmartPointer<Type> _M_value;

    public:
        ReferenceWrapper() = default;
        ReferenceWrapper(Type& value)
        {
            _M_value = SmartPointer<Type>(&value, fake_delete<Type>);
        }

        ReferenceWrapper(const Type& value)
        {
            _M_value = SmartPointer<Type>(new Type(value), delete_value<Type>);
        }

        ReferenceWrapper(const ReferenceWrapper&) = default;

        ReferenceWrapper& operator=(const Type& value)
        {
            if (_M_value.get() == nullptr)
                _M_value = SmartPointer<Type>(new Type(value), delete_value<Type>);
            else
                *_M_value = value;
            return *this;
        }

        ReferenceWrapper& operator=(const ReferenceWrapper& wrapper)
        {
            _M_value = wrapper._M_value;
            return *this;
        }

        ReferenceWrapper(ReferenceWrapper&&) = default;
        ReferenceWrapper& operator = (ReferenceWrapper&&) = default;

        Type& get() const
        {
            if (_M_value.get() == nullptr)
                throw std::runtime_error("Null ref exception");
            return *_M_value.get();
        }

        operator Type()
        {
            return get();
        }

        operator Type&()
        {
            return get();
        }

        bool is_null() const
        {
            return _M_value.get() == nullptr;
        }

        std::size_t references() const
        {
            return _M_value.use_count();
        }
    };
}// namespace Engine
