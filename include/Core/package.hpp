#pragma once

#include <Core/callback.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>


namespace Engine
{

    class ENGINE_EXPORT Package : public Object
    {
        declare_class(Package, Object);

    public:
        using ObjectMap = Map<Name, Object*, Name::HashFunction>;


    private:
        ObjectMap m_objects;

        Object* find_object_private_no_recurse(const StringView& name) const;
        Object* find_object_private(StringView name) const;


    public:
        delete_copy_constructors(Package);
        Package();

        bool add_object(Object* object, bool autorename = false);
        Package& remove_object(Object* object);
        Object* find_object(const StringView& name, bool recursive = true) const;

        const ObjectMap& objects() const;
        bool contains_object(const Object* object) const;
        bool contains_object(const StringView& name) const;

        bool is_engine_resource() const override;
        bool save(BufferWriter* writer = nullptr, Flags<SerializationFlags> flags = {}) override;

        template<typename Type>
        FORCE_INLINE Type* find_object_checked(const StringView& object_name, bool recursive = true) const
        {
            Object* object = Package::find_object(object_name, recursive);
            if (object)
                return object->instance_cast<Type>();
            return nullptr;
        }

        ~Package();
        friend class Object;
    };
}// namespace Engine
