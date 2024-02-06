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
        ObjectMap _M_objects;

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

        bool save(BufferWriter* writer = nullptr) const;
        bool save(const Path& path) const;

        bool load(BufferReader* reader = nullptr, Flags flags = 0);
        bool load(const Path& path, Flags flags = 0);
        Object* load_object(const StringView& name, Flags flags = 0, BufferReader* reader = nullptr);
        Object* load_object(const Path& file_path, const StringView& name, Flags flags = 0);

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
