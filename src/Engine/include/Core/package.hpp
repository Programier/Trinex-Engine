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
        using Filter = CallBack<bool(Object*)>;

    private:
        Vector<Object*> _M_objects;
        Object* get_object_by_name(const String& name) const;
        Pair<void*, BufferReader*>* _M_loader_data = nullptr;
        bool load_entry(void* entry, class Archive*);
        bool load_buffer(BufferReader* reader, Vector<char>& original_buffer);
        CallBacks<bool(Object*)> _M_filters;

        Index lower_bound_of(HashIndex hash) const;
        Index lower_bound_of(const Object* object) const;
        Index lower_bound_of(const String& name) const;
        Index validate_index(Index index, HashIndex hash) const;


    public:
        delete_copy_constructors(Package);
        Package();
        Package(const String& name);

        bool add_object(Object* object, bool autorename = false);
        Package& remove_object(Object* object);
        Object* find_object(const String& name, bool recursive = true) const;
        const Vector<Object*>& objects() const;
        bool can_destroy(MessageList& messages) override;
        bool save(BufferWriter* writer = nullptr) const;
        bool load(BufferReader* read = nullptr, bool clear = false);
        Object* load_object(const String& name, BufferReader* reader = nullptr);
        Identifier add_filter(const Filter& filter);
        Package& remove_filter(Identifier id);
        const CallBacks<bool(Object*)>& filters() const;
        bool contains_object(const Object* object) const;
        bool contains_object(const String& name) const;


        template<typename Type>
        Type* find_object_checked(const String& name, bool recursive = true)
        {
            Object* object = Package::find_object(name, recursive);
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
