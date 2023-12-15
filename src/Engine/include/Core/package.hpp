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

        Object* find_object_private_no_recurce(const char* name, size_t name_len) const;
        Object* find_object_private(const char* name, size_t name_len) const;

    public:
        delete_copy_constructors(Package);
        Package();

        bool add_object(Object* object, bool autorename = false);
        Package& remove_object(Object* object);
        Object* find_object(const String& name, bool recursive = true) const;
        Object* find_object(const char* name, bool recursive = true) const;
        Object* find_object(const char* name, size_t name_len, bool recursive = true) const;

        const ObjectMap& objects() const;
        bool save(BufferWriter* writer = nullptr) const;
        bool load(BufferReader* read = nullptr, bool clear = false);
        Object* load_object(const String& name, BufferReader* reader = nullptr);
        const CallBacks<bool(Object*)>& filters() const;
        bool contains_object(const Object* object) const;
        bool contains_object(const String& name) const;

        template<typename Type>
        FORCE_INLINE Type* find_object_checked(const String& object_name, bool recursive = true) const
        {
            Object* object = Package::find_object(object_name, recursive);
            if (object)
                return object->instance_cast<Type>();
            return nullptr;
        }

        template<typename Type>
        FORCE_INLINE Type* find_object_checked(const char* object_name, bool recursive = true) const
        {
            Object* object = Package::find_object(object_name, recursive);
            if (object)
                return object->instance_cast<Type>();
            return nullptr;
        }

        template<typename Type>
        FORCE_INLINE Type* find_object_checked(const char* object_name, size_t name_len, bool recursive = true) const
        {
            Object* object = Package::find_object(object_name, name_len, recursive);
            if (object)
                return object->instance_cast<Type>();
            return nullptr;
        }

        template<typename CurrentClass>
        static void initialize_script_bindings(class Class* registrable_class)
        {
            Super::initialize_script_bindings<CurrentClass>(registrable_class);
            ScriptClassRegistrar registrar = registrable_class;
            registrar.method("bool add_object(Object@, bool)", &CurrentClass::add_object)
                    .method("Package& remove_object(Object@)", &CurrentClass::remove_object);
        }

        ~Package();
        friend class Object;
    };
}// namespace Engine
