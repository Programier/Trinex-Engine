#pragma once

#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>


namespace Engine
{
    class ENGINE_EXPORT Package : public Object
    {
    public:
        using ObjectMap = Map<String, Object*>;

    private:
        ObjectMap _M_objects;
        Object* get_object_by_name(const String& name) const;

    public:
        delete_copy_constructors(Package);
        Package();
        Package(const String& name);

        bool add_object(Object* object);
        Package& remove_object(Object* object);
        Object* find_object_in_package(const String& name, bool recursive = true) const;
        const ObjectMap& objects() const;
        bool can_destroy(MessageList& messages) override;
        bool save(BufferWriter* writer = nullptr) const;
        bool load(BufferReader* read = nullptr, bool clear = false);

        template<typename Type>
        Type* find_object_checked_in_package(const String& name, bool recursive = true)
        {
            Object* object = find_object_in_package(name, recursive);
            if (object)
                return object->instance_cast<Type>();
            return nullptr;
        }

        ~Package();
        friend class Object;
    };
}// namespace Engine