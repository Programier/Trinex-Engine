#pragma once

#include <Core/implement.hpp>
#include <Core/object.hpp>


namespace Engine
{
    class ENGINE_EXPORT Package : public Object
    {
    public:
        using ObjectMap = Map<String, Object*>;
        using Super = Object;

    private:
        ObjectMap _M_objects;
        Object* get_object_by_name(const String& name) const;
        Pair<void*, BufferReader*>* _M_loader_data = nullptr;
        bool load_entry(void* entry, class Archive*);
        bool load_buffer(BufferReader* reader,  Vector<char>& original_buffer);

    public:
        delete_copy_constructors(Package);
        Package();
        Package(const String& name);

        bool add_object(Object* object, bool autorename = false);
        Package& remove_object(Object* object);
        Object* find_object(const String& name, bool recursive = true) const;
        const ObjectMap& objects() const;
        bool can_destroy(MessageList& messages) override;
        bool save(BufferWriter* writer = nullptr) const;
        bool load(BufferReader* read = nullptr, bool clear = false);
        Object* load_object(const String& name, BufferReader* reader = nullptr);

        template<typename Type>
        Type* find_object_checked(const String& name, bool recursive = true)
        {
            Object* object = Package::find_object(name, recursive);
            if (object)
                return object->instance_cast<Type>();
            return nullptr;
        }

        ~Package();
        friend class Object;
    };
}// namespace Engine
