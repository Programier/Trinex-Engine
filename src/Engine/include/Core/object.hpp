#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Core/object_types.hpp>
#include <string>
#include <typeinfo>
#include <unordered_set>

namespace Engine
{
    class Package;
    using ObjectSet = std::unordered_set<class Object*>;

    // Head of all classes in the Engine
    CLASS Object
    {
    public:
        struct InstanceInfo {
            std::size_t _M_offset = 0;
            bool _M_has_instance = false;
        };

    private:
        mutable bool _M_need_delete = false;
        mutable String _M_name;

        bool _M_is_on_heap = false;
        BitMask _M_flags;
        Object* _M_parent = nullptr;
        ObjectSet _M_childs;

        void delete_instance(bool force_delete = false) ;
        Package* _M_package = nullptr;

    protected:
        Object();
        Object& mark_as_on_heap_instance();

    public:
        InstanceInfo _M_instance_info[static_cast<std::size_t>(ObjectType::Count)];

    public:
        delete_copy_constructors(Object);

        ENGINE_EXPORT static String decode_name(const std::type_info& info);
        ENGINE_EXPORT static String decode_name(const String& name);
        String class_name() const;
        Object& add_child_object(Object * child);
        Object& remove_child_object(Object * child);
        const ObjectSet& child_objects() const;
        Object* parent_object() const;
        Object& parent_object(Object * parent);
        std::size_t class_hash() const;
        ENGINE_EXPORT static const ObjectSet& all_objects();
        bool mark_for_delete();
        bool is_on_heap() const;
        ENGINE_EXPORT static void collect_garbage();
        BitMask flags() const;
        Object& add_flags(BitMask flags);
        Object& remove_flags(BitMask flags);
        bool has_any_flags(BitMask flags) const;
        bool has_all_flags(BitMask flags) const;
        const String& name() const;
        Object& name(const String& name);
        virtual Object& copy(const Object* copy_from);
        Object& add_to_package(Package * package);
        Object& remove_from_package();
        Package* package() const;
        String full_name() const;
        ENGINE_EXPORT static Object* find_object(const String& object_name);


        template<typename Type, typename... Args>
        inline static Type* new_instance(const Args&... args)
        {
            auto instance = new (::operator new(sizeof(Type))) Type(args...);
            instance->mark_as_on_heap_instance();
            return instance;
        }

        template<typename Type>
        inline static std::size_t instance_hash_of()
        {
            return typeid(Type).hash_code();
        }

        template<typename ObjectInstanceType>
        typename std::enable_if<static_cast<EnumerateType>(ObjectInstanceType::instance_type) != 0, bool>::type is_instance_of() const
        {
            return _M_instance_info[static_cast<EnumerateType>(ObjectInstanceType::instance_type)]._M_has_instance;
        }

        template<typename ObjectInstanceType>
        typename std::enable_if<static_cast<EnumerateType>(ObjectInstanceType::instance_type) != 0, ObjectInstanceType*>::type
        instance_cast()
        {
            const EnumerateType index = static_cast<EnumerateType>(ObjectInstanceType::instance_type);
            if (_M_instance_info[index]._M_has_instance)
            {
                byte* new_object = reinterpret_cast<byte*>(this) - _M_instance_info[index]._M_offset;
                return reinterpret_cast<ObjectInstanceType*>(new_object);
            }

            return nullptr;
        }

        template<typename ObjectInstanceType>
        typename std::enable_if<static_cast<EnumerateType>(ObjectInstanceType::instance_type) != 0, const ObjectInstanceType*>::type
        instance_cast() const
        {
            const EnumerateType index = static_cast<EnumerateType>(ObjectInstanceType::instance_type);
            if (_M_instance_info[index]._M_has_instance)
            {
                const byte* new_object = reinterpret_cast<const byte*>(this) - _M_instance_info[index]._M_offset;
                return reinterpret_cast<ObjectInstanceType*>(new_object);
            }

            return nullptr;
        }

        template<typename ObjectInstanceType>
        static typename std::enable_if<static_cast<EnumerateType>(ObjectInstanceType::instance_type) != 0, Object*>::type
        find_object_checked(const String& object_name)
        {
            Object* object = find_object(object_name);
            if (object)
            {
                return object->instance_cast<ObjectInstanceType>();
            }
            return nullptr;
        }

        ENGINE_EXPORT static void* operator new(std::size_t size);
        ENGINE_EXPORT static void* operator new[](std::size_t count);
        ENGINE_EXPORT static void* operator new(std::size_t size, void* data);
        ENGINE_EXPORT static void* operator new[](std::size_t size, void* data);
        ENGINE_EXPORT static const Package* root_package();


        virtual ~Object();
        friend void force_garbage_collection();
        friend class Package;
    };

    struct __FOR_PRIVATE_USAGE__ {
    };

}// namespace Engine
