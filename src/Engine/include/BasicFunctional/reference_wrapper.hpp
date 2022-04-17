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
            _M_value = &value;
            _M_value.delete_value(false);
        }

        ReferenceWrapper(const Type& value)
        {
            _M_value.delete_value(true) = new Type(value);
        }

        ReferenceWrapper& operator=(const Type& value)
        {
            if (_M_value.get() == nullptr)
            {
                _M_value.delete_value(true) = new Type(value);
            }
            else
                *_M_value = value;
            return *this;
        }

        ReferenceWrapper& operator=(const ReferenceWrapper& wrapper)
        {
            _M_value = wrapper._M_value;
            return *this;
        }

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
    };
}// namespace Engine
