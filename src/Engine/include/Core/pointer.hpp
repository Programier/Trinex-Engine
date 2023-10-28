#pragma once

#include <Core/etl/stl_wrapper.hpp>
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
        union
        {
            InstanceClass* _M_instance = nullptr;
            Object* _M_object;
        };


    public:
        struct HashStruct : public Hash<InstanceClass*> {
            size_t operator()(const Pointer<InstanceClass>& instance) const
            {
                return static_cast<Hash<InstanceClass*>>(*this)(instance._M_instance);
            }
        };

        Pointer(InstanceClass* instance = nullptr) : _M_instance(instance)
        {
            add_reference(_M_object);
        }

        Pointer(const Pointer& pointer)
        {
            *this = pointer;
        }

        Pointer(Pointer&& pointer)
        {
            _M_instance         = pointer._M_instance;
            pointer._M_instance = nullptr;
        }

        Pointer& operator=(const Pointer& pointer)
        {
            if (this == &pointer)
                return *this;

            remove_reference(_M_object);
            _M_instance = pointer._M_instance;
            add_reference(_M_object);
            return *this;
        }

        Pointer& operator=(Pointer&& pointer)
        {
            if (this != &pointer)
            {
                _M_instance         = pointer._M_instance;
                pointer._M_instance = nullptr;
            }

            return *this;
        }

        Pointer& operator=(InstanceClass* instance)
        {
            remove_reference(_M_object);
            _M_instance = instance;
            add_reference(_M_object);
            return *this;
        }


        InstanceClass* operator->() const
        {
            return _M_instance;
        }

        operator InstanceClass*()
        {
            return _M_instance;
        }

        operator const InstanceClass*() const
        {
            return _M_instance;
        }


        InstanceClass* ptr() const
        {
            return _M_instance;
        }

        bool operator==(const Pointer<InstanceClass>& instance) const
        {
            return _M_instance == instance._M_instance;
        }

        bool operator!=(const Pointer<InstanceClass>& instance) const
        {
            return _M_instance != instance._M_instance;
        }

        bool operator==(const InstanceClass* instance) const
        {
            return _M_instance == instance;
        }

        bool operator!=(const InstanceClass* instance) const
        {
            return _M_instance != instance;
        }

        ~Pointer()
        {
            remove_reference(_M_object);
        }
    };
}// namespace Engine
