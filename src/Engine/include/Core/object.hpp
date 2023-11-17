#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/implement.hpp>
#include <Core/memory_manager.hpp>
#include <Core/name.hpp>
#include <Core/serializable_object.hpp>
#include <ScriptEngine/registrar.hpp>

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
        mutable BitSet<static_cast<size_t>(TrinexObjectFlags::__OF_COUNT__)> _M_trinex_flags;


        BitSet<static_cast<size_t>(ObjectFlags::__OF_COUNT__)> _M_flags;
        Package* _M_package;
        Counter _M_references;
        Index _M_index_in_package;
        Name _M_name;
        mutable Index _M_instance_index;

    private:
        void delete_instance();
        static void create_default_package();
        static bool object_is_exist(Package* package, const String& name);

        const Object& remove_from_instances_array() const;

        template<typename Type, typename... Args>
        static Type* allocate_new_instance(Args&&... args)
        {
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                return new (MemoryManager::instance().find_memory<Type>()) Type(std::forward<Args>(args)...);
            }
            else
            {
                return new Type(std::forward<Args>(args)...);
            }
        }

        static void private_bind_class(class Class* c);


    protected:
        PriorityIndex _M_force_destroy_priority = 0;

        Object();
        Object& mark_as_allocate_by_constroller();
        Object& insert_to_default_package();
        bool private_check_instance(const class Class* const check_class) const;
        bool trinex_flag(TrinexObjectFlags flag) const;
        const Object& trinex_flag(TrinexObjectFlags flag, bool status) const;

        static Object* noname_object();


    protected:
        static class Class* _M_static_class;


    public:
        using This  = Object;
        using Super = Object;
        static Object* static_constructor();
        static void static_initialize_class();
        static class Class* static_class_instance();
        static HashIndex hash_of_name(const String& name);
        virtual class Class* class_instance() const;

        template<typename CurrentClass>
        static void initialize_script_bindings(class Class* registrable_class)
        {
            private_bind_class(registrable_class);
        }

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
        const String& string_name() const;
        HashIndex hash_index() const;
        ObjectRenameStatus name(String name, bool autorename = false);
        virtual Object* copy();
        bool add_to_package(Package* package, bool autorename = false);
        Object& remove_from_package();
        Package* package() const;
        String full_name() const;
        Counter references() const;
        void add_reference();
        void remove_reference();
        const decltype(Object::_M_flags)& flags() const;
        Object& flag(ObjectFlags flag, bool status);
        bool flag(ObjectFlags flag) const;
        bool is_noname() const;

        ENGINE_EXPORT static Object* find_object(const String& object_name);
        static Package* find_package(const String& name, bool create = true);

        virtual bool can_destroy(MessageList& messages);
        virtual Object& preload();
        virtual Object& postload();

        String as_string() const;
        Index instance_index() const;

        bool archive_process(Archive* archive) override;

        // NOTE! You will manually push object to package, if you use this method!
        template<typename Type, typename... Args>
        static Type* new_instance_without_package(Args&&... args)
        {
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                if constexpr (is_singletone_v<Type>)
                {
                    if (Type::instance() != nullptr)
                    {
                        return Type::instance();
                    }
                }

                Type* instance = allocate_new_instance<Type>(std::forward<Args>(args)...);
                instance->mark_as_allocate_by_constroller();
                return instance;
            }
            else
            {
                return allocate_new_instance<Type>(std::forward<Args>(args)...);
            }
        }

        template<typename Type, typename... Args>
        static Type* new_instance(Args&&... args)
        {
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                if constexpr (is_singletone_v<Type>)
                {
                    if (Type::instance() != nullptr)
                    {
                        return Type::instance();
                    }
                }

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
                if constexpr (is_singletone_v<Type>)
                {
                    if (Type::instance() != nullptr)
                    {
                        return Type::instance();
                    }
                }

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
            return private_check_instance(Type::static_class_instance());
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
        static ENGINE_EXPORT void force_garbage_collection();
        friend void call_force_garbage_collection();


    public:
        ENGINE_EXPORT static Package* root_package();


        virtual ~Object();
        friend class PointerBase;
        friend class Package;
        friend class Archive;
        friend class MemoryManager;
        friend class EngineInstance;
    };


#define declare_class(class_name, base_name)                                                                           \
protected:                                                                                                             \
    static class Class* _M_static_class;                                                                               \
                                                                                                                       \
public:                                                                                                                \
    using This  = class_name;                                                                                          \
    using Super = base_name;                                                                                           \
    static Object* static_constructor();                                                                               \
    static void static_initialize_class();                                                                             \
    static class Class* static_class_instance();                                                                       \
    virtual class Class* class_instance() const override;                                                              \
                                                                                                                       \
private:

#define implement_initialize_class(name) void name::static_initialize_class()

#define implement_default_initialize_class(name)                                                                       \
    implement_initialize_class(name)                                                                                   \
    {}

#define implement_class(class_name, namespace_name)                                                                    \
    class Class* class_name::_M_static_class = nullptr;                                                                \
    Object* class_name::static_constructor()                                                                           \
    {                                                                                                                  \
        if constexpr (std::is_abstract_v<class_name>)                                                                  \
        {                                                                                                              \
            return nullptr;                                                                                            \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            return Engine::Object::new_instance<class_name>();                                                         \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    class Class* class_name::class_instance() const                                                                    \
    {                                                                                                                  \
        return class_name::static_class_instance();                                                                    \
    }                                                                                                                  \
                                                                                                                       \
    class Class* class_name::static_class_instance()                                                                   \
    {                                                                                                                  \
        if (!_M_static_class)                                                                                          \
        {                                                                                                              \
            bool has_base_class        = &This::_M_static_class != &Super::_M_static_class;                            \
            String class_instance_name = namespace_name;                                                               \
            if (!class_instance_name.empty())                                                                          \
            {                                                                                                          \
                class_instance_name += "::";                                                                           \
            }                                                                                                          \
            class_instance_name += #class_name;                                                                        \
            _M_static_class = new Class(class_instance_name, &This::static_constructor,                                \
                                        has_base_class ? Super::static_class_instance() : nullptr);                    \
            _M_static_class->process_type<class_name>();                                                               \
            class_name::static_initialize_class();                                                                     \
            if constexpr (This::initialize_script_bindings<This> != Super::initialize_script_bindings<This>)           \
            {                                                                                                          \
                This::initialize_script_bindings<This>(_M_static_class);                                               \
                Engine::ScriptClassRegistrar::global_namespace_name("");                                               \
            }                                                                                                          \
        }                                                                                                              \
        return _M_static_class;                                                                                        \
    }                                                                                                                  \
    static InitializeController pre_initialize_##class_name([]() { class_name::static_class_instance(); },             \
                                                            "Initialize " namespace_name #class_name);


#define implement_class_default_init(class_name, namespace_name)                                                       \
    implement_class(class_name, namespace_name);                                                                       \
    implement_default_initialize_class(class_name)

#define implement_engine_class(class_name) implement_class(class_name, "Engine")
#define implement_engine_class_default_init(class_name) implement_class_default_init(class_name, "Engine")
}// namespace Engine
