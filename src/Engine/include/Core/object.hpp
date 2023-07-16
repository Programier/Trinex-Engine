#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/metadata.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/implement.hpp>
#include <Core/memory_manager.hpp>
#include <Core/serializable_object.hpp>
#include <string>
#include <typeinfo>

namespace Engine
{

    class Package;
    class Object;
    using MessageList = List<String>;

    enum TrinexObjectFlags : size_t
    {
        None = 0,
        IsOnHeap,
        IsDestructed,
        IsNeedDelete,
        IsCollectedByGC,
        IsSerializable,
        IsAllocatedByController,
        IsUnregistered,
        IsPackage,
        __OF_COUNT__
    };

    enum class ObjectFlags : size_t
    {
        OF_LoadWithDependencies = 0,
        __OF_COUNT__
    };

    enum class ObjectRenameStatus
    {
        Success,
        Skipped,
        Failed,
    };

    // Head of all classes in the Engine
    class ENGINE_EXPORT Object : public SerializableObject
    {
    public:
        using ObjectClass = Object;

    private:
        mutable String _M_name;
        mutable BitSet<static_cast<size_t>(TrinexObjectFlags::__OF_COUNT__)> _M_trinex_flags;
        BitSet<static_cast<size_t>(ObjectFlags::__OF_COUNT__)> _M_flags;

        Package* _M_package                 = nullptr;
        mutable const class Class* _M_class = nullptr;
        Counter _M_references               = 0;
        mutable Index _M_instance_index             = static_cast<Index>(~0U);


        void delete_instance();
        static void create_default_package();
        static bool object_is_exist(Package* package, const String& name);

        const Object& remove_from_instances_array() const;

    protected:
        PriorityIndex _M_force_destroy_priority = 0;

        Object();
        Object& mark_as_allocate_by_constroller();
        Object& insert_to_default_package();
        bool private_check_instance(const class Class* const check_class) const;
        bool trinex_flag(TrinexObjectFlags flag) const;
        const Object& trinex_flag(TrinexObjectFlags flag, bool status) const;

    public:
        delete_copy_constructors(Object);
        ENGINE_EXPORT static String decode_name(const std::type_info& info);
        ENGINE_EXPORT static String decode_name(const String& name);
        ENGINE_EXPORT static Package* load_package(const String& name);
        ENGINE_EXPORT static Object* load_object(const String& name);
        ENGINE_EXPORT static String package_name_of(const String& name);
        ENGINE_EXPORT static String object_name_of(const String& name);
        String decode_name() const;
        ENGINE_EXPORT static const ObjectArray& all_objects();
        bool mark_for_delete(bool skip_check = false);
        bool is_on_heap() const;
        ENGINE_EXPORT static void collect_garbage();
        const String& name() const;
        ObjectRenameStatus name(String name, bool autorename = false);
        virtual Object* copy();
        bool add_to_package(Package* package, bool autorename = false);
        Object& remove_from_package();
        Package* package() const;
        String full_name() const;
        Counter references() const;
        const decltype(Object::_M_flags)& flags() const;
        Object& flag(ObjectFlags flag, bool status);
        bool flag(ObjectFlags flag) const;

        ENGINE_EXPORT static Object* find_object(const String& object_name);
        virtual bool can_destroy(MessageList& messages);
        virtual void post_init_components();
        static Package* find_package(const String& name, bool create = true);
        const class Class* class_instance() const;
        String as_string() const;
        Index instance_index() const;

        bool archive_process(Archive* archive) override;


        // NOTE! You will manually push object to package, if you use this method!
        template<typename Type, typename... Args>
        static Type* new_instance_without_package(Args&&... args)
        {
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                Type* instance = new (MemoryManager::instance().find_memory<Type>()) Type(std::forward<Args>(args)...);
                instance->mark_as_allocate_by_constroller();
                instance->_M_class = const_cast<const Class*>(ClassMetaData<Type>::find_class());
                return instance;
            }
            else
            {
                return new Type(args...);
            }
        }

        template<typename Type, typename... Args>
        static Type* new_instance(Args&&... args)
        {
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                Type* instance = new_instance_without_package<Type>(std::forward<Args>(args)...);
                instance->insert_to_default_package();
                return instance;
            }
            else
            {
                return new Type(args...);
            }
        }

        template<typename Type, typename... Args>
        static Type* new_instance_named(const String& name, Package* package = nullptr, Args&&... args)
        {
            if constexpr (!std::is_base_of_v<Object, Type>)
            {
                return nullptr;
            }
            else
            {

                if (package == nullptr)
                {
                    package = root_package();
                }

                if (object_is_exist(package, name))
                {
                    return nullptr;
                }

                Type* instance = new_instance_without_package<Type>(std::forward<Args>(args)...);

                instance->name(name);

                if (!instance->add_to_package(package))
                {
                    delete instance;
                    instance = nullptr;
                }

                return instance;
            }
        }

        template<typename Type>
        inline static std::size_t instance_hash_of()
        {
            return typeid(Type).hash_code();
        }

        template<typename Type>
        typename std::enable_if<is_object_based<Type>::value, bool>::type is_instance_of() const
        {
            return private_check_instance(ClassMetaData<Type>::find_class());
        }

        template<typename Type>
        typename std::enable_if<!is_object_based<Type>::value, bool>::type is_instance_of() const
        {
            return false;
        }

        template<typename Type>
        typename std::enable_if<is_object_based<Type>::value, const Type*>::type instance_cast() const
        {
            if (!is_instance_of<Type>())
                return nullptr;
            return (const Type*) this;
        }

        template<typename Type>
        typename std::enable_if<is_object_based<Type>::value, Type*>::type instance_cast()
        {
            if (!is_instance_of<Type>())
                return nullptr;
            return (Type*) this;
        }

        template<typename Type>
        typename std::enable_if<!is_object_based<Type>::value, const Type*>::type instance_cast() const
        {
            return nullptr;
        }

        template<typename Type>
        typename std::enable_if<!is_object_based<Type>::value, Type*>::type instance_cast()
        {
            return nullptr;
        }

        template<typename ObjectInstanceType>
        static ObjectInstanceType* find_object_checked(const String& object_name)
        {
            Object* object = find_object(object_name);
            if (object)
            {
                return object->instance_cast<ObjectInstanceType>();
            }
            return nullptr;
        }

        template<typename Type>
        static void begin_destroy(const Type*& instance)
        {
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                if (instance->trinex_flag(TrinexObjectFlags::IsOnHeap))
                {
                    instance->mark_for_delete();
                }
            }
            else
            {
                delete instance;
            }
        }

        template<typename Type>
        static void begin_destroy(Type*& instance)
        {
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                if (instance->trinex_flag(TrinexObjectFlags::IsOnHeap))
                {
                    instance->mark_for_delete();
                }
            }
            else
            {
                delete instance;
            }

            instance = nullptr;
        }

    private:
        static void* operator new(std::size_t size) = delete;
        static void* operator new(std::size_t size, void* data);
        static void* operator new[](std::size_t count)            = delete;
        static void* operator new[](std::size_t size, void* data) = delete;

        static ENGINE_EXPORT void force_garbage_collection();
        friend void call_force_garbage_collection();

    protected:
        static void operator delete(void* data);
        static void operator delete[](void* data) = delete;

    public:
        ENGINE_EXPORT static Package* root_package();


        virtual ~Object();
        friend class Package;
        friend class PointerBase;
        friend class Archive;
        friend class MemoryManager;
        friend class EngineInstance;
    };

    template<typename Return, typename... Args>
    Return (*func_of(Return (*function)(Args...)))(Args...)
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    Return (Instance::*func_of(Return (Instance::*function)(Args...)))(Args...)
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    Return (Instance::*func_of(Return (Instance::*function)(Args...) const))(Args...) const
    {
        return function;
    }

#define TRINEX_OBJECT_HEADER_INCLUDED 1
}// namespace Engine
