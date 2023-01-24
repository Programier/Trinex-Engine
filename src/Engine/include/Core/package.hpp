#pragma once

#include <Core/export.hpp>
#include <Core/object.hpp>
#include <Core/implement.hpp>
#include <unordered_map>

namespace Engine
{
    ENGINE_EXPORT class Package : public Object
    {
    public:
        using ObjectMap = std::unordered_map<String, Object*>;
    private:
        ObjectMap _M_objects;
        Object* get_object_by_name(const String& name) const;


        declare_instance_info_hpp(Package);
    public:
        delete_copy_constructors(Package);
        constructor_hpp(Package);
        constructor_hpp(Package, const String& name);

        Package& add_object(Object* object);
        Package& remove_object(Object* object);
        Object* find_object_in_package(const String& name, bool recursive = true) const;
        const ObjectMap& objects() const;
    };
}
