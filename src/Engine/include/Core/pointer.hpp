#pragma once

#include <Core/export.hpp>

namespace Engine
{
    class Object;

    class ENGINE_EXPORT PointerBase
    {
    protected:
        PointerBase& add_reference(Object* object);
        PointerBase& remove_reference(Object* object);
        PointerBase();
    };

    template<class InstanceClass>
    class Pointer : private PointerBase
    {
    private:
        InstanceClass* _M_instance = nullptr;


    public:
        Pointer(InstanceClass* instance = nullptr) : _M_instance(instance)
        {
            add_reference(_M_instance);
        }

        Pointer(const Pointer& pointer)
        {
            *this = pointer;
        }

        Pointer(Pointer&& pointer)
        {
            _M_instance = pointer._M_instance;
            pointer._M_instance = nullptr;
        }

        Pointer& operator=(const Pointer& pointer)
        {
            if (this == &pointer)
                return *this;

            remove_reference(_M_instance);
            _M_instance = pointer._M_instance;
            add_reference(_M_instance);
        }

        Pointer& operator=(Pointer&& pointer)
        {
            if (this != &pointer)
            {
                _M_instance = pointer._M_instance;
                pointer._M_instance = nullptr;
            }

            return *this;
        }

        Pointer& operator=(InstanceClass* instance)
        {
            remove_reference(_M_instance);
            _M_instance = instance;
            add_reference(_M_instance);
            return *this;
        }

        InstanceClass* operator->()
        {
            return _M_instance;
        }

        operator InstanceClass*()
        {
            return _M_instance;
        }

        ~Pointer()
        {
            remove_reference(_M_instance);
        }
    };
}// namespace Engine
