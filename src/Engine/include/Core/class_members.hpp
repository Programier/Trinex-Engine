#pragma once
#include <Core/buffer_manager.hpp>
#include <Core/exception.hpp>
#include <Core/object.hpp>
#include <cstring>

namespace Engine
{
    using ClassFieldOffset = size_t;

    class ENGINE_EXPORT ClassField : public Object
    {
    private:
        ClassFieldOffset _M_offset;
        AccessType _M_access_type;
        bool _M_is_serializable;

        bool (ClassField::*_M_archive_process)(Archive* ar, Object* object);

        ClassField(ClassFieldOffset offset, AccessType access_type = AccessType::Public, bool is_serializable = true);

        template<typename PropType>
        bool private_archive_process(Archive* ar, Object* object)
        {
            PropType* prop = from<PropType>(object);
            if (prop == nullptr)
            {
                return false;
            }

            return static_cast<bool>((*ar) & (*prop));
        }

    public:
        using Super = Object;

        template<typename PropType, typename InstanceType>
        ClassField(PropType InstanceType::*prop, AccessType access_type = AccessType::Public,
                   bool is_serializable = true)
            : ClassField(*reinterpret_cast<ClassFieldOffset*>(&prop), access_type, is_serializable)
        {
            _M_archive_process = &ClassField::private_archive_process<PropType>;
        }

        ClassFieldOffset offset() const;

        byte* from(Object* object) const;
        const byte* from(const Object* object) const;
        AccessType access_type() const;
        bool is_serializable() const;
        bool archive_process_from(Archive* ar, Object* object);

        template<typename Type>
        Type* from(Object* object)
        {
            return reinterpret_cast<Type*>(from(object));
        }

        template<typename Type>
        const Type* from(const Object* object)
        {
            return reinterpret_cast<const Type*>(from(object));
        }

        template<typename Type>
        Type& ref_from(Object* object)
        {
            Type* data = reinterpret_cast<Type*>(from(object));
            if (data)
            {
                return *data;
            }

            throw EngineException("Cannot get field from object!");
        }

        template<typename Type>
        const Type& ref_from(const Object* object)
        {
            Type* data = reinterpret_cast<Type*>(from(object));
            if (data)
            {
                return *data;
            }

            throw EngineException("Cannot get field from object!");
        }
    };
}// namespace Engine
