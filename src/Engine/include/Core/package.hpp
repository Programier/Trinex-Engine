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

        struct HeaderEntry {
            Vector<Name> class_hierarchy;
            size_t offset            = 0;
            size_t uncompressed_size = 0;
            Object* object           = nullptr;
            Name object_name;

            Vector<byte> compressed_data;
        };

        using Header = Vector<HeaderEntry>;

    private:
        ObjectMap _M_objects;
        Header _M_header;

        Object* find_object_private_no_recurce(const StringView& name) const;
        Object* find_object_private(StringView name) const;


        const Package& build_header(Header& header) const;
        Header& load_header_private(class BufferReader* reader);

    public:
        delete_copy_constructors(Package);
        Package();

        bool add_object(Object* object, bool autorename = false);
        Package& remove_object(Object* object);
        Object* find_object(const StringView& name, bool recursive = true) const;

        const ObjectMap& objects() const;
        const Header& header() const;
        bool contains_object(const Object* object) const;
        bool contains_object(const StringView& name) const;

        bool save() const;
        bool load(BufferReader* reader = nullptr);
        Header& load_header();

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

    ENGINE_EXPORT bool operator&(Archive& ar, Package::HeaderEntry& entry);
}// namespace Engine
