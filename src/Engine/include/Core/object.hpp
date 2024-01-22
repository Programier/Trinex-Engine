#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/implement.hpp>
#include <Core/name.hpp>
#include <Core/serializable_object.hpp>
#include <ScriptEngine/registrar.hpp>

#include <typeinfo>

namespace Engine
{
    class Package;
    class Object;
    using MessageList = List<String>;


    ENGINE_EXPORT const char* operator""_localized(const char* line, size_t len);

    enum class ObjectRenameStatus
    {
        Success,
        Skipped,
        Failed,
    };

    enum class GCFlag
    {
        OnlyMarked      = 0,
        FindUnreacheble = 1,
        DestroyAll      = 2,
    };


    // Head of all classes in the Engine
    class ENGINE_EXPORT Object : public SerializableObject
    {
    public:
        using ObjectClass = Object;


        enum Flag : BitMask
        {
            None             = 0,
            IsDestructed     = (1 << 0),
            IsSerializable   = (1 << 1),
            IsAvailableForGC = (1 << 2),
            IsPackage        = (1 << 3),
            IsUnreachable    = (1 << 4),
            IsEditable       = (1 << 5),
        };

    private:
        Package* _M_package;
        Object* _M_owner;
        Counter _M_references;
        Name _M_name;
        mutable Index _M_instance_index;

    private:
        static void create_default_package();
        static bool object_is_exist(Package* package, const String& name);

        const Object& remove_from_instances_array() const;
        static void prepare_next_object_for_gc();

    protected:
        Object();
        bool private_check_instance(const class Class* const check_class) const;
        static Object* noname_object();


    protected:
        static class Class* _M_static_class;


    public:
        mutable Flags flags;

        using This  = Object;
        using Super = Object;
        static Object* static_constructor();
        static void static_initialize_class();
        static class Class* static_class_instance();
        static HashIndex hash_of_name(const String& name);
        virtual class Class* class_instance() const;

        delete_copy_constructors(Object);
        ENGINE_EXPORT static Package* load_package(const String& name);
        ENGINE_EXPORT static String package_name_of(const String& name);
        ENGINE_EXPORT static String object_name_of(const String& name);
        ENGINE_EXPORT static const ObjectArray& all_objects();
        ENGINE_EXPORT static Object* find_object(const String& object_name);
        ENGINE_EXPORT static Object* find_object(const char* object_name);
        ENGINE_EXPORT static Object* find_object(const char* object_name, size_t len);
        ENGINE_EXPORT static Package* root_package();

        ENGINE_EXPORT static void collect_garbage(GCFlag flag = GCFlag::OnlyMarked);
        ENGINE_EXPORT static const String& language();
        ENGINE_EXPORT static void language(const String& new_language);
        ENGINE_EXPORT static void language(const char* new_language);
        ENGINE_EXPORT static const String& localize(const String& line);
        ENGINE_EXPORT static const String& localize(const char* line);

        const String& string_name() const;
        HashIndex hash_index() const;
        ObjectRenameStatus name(const char* name, size_t name_len, bool autorename = false);
        ObjectRenameStatus name(const char* name, bool autorename = false);
        ObjectRenameStatus name(const String& name, bool autorename = false);
        const Name& name() const;
        virtual Object* copy();
        bool add_to_package(Package* package, bool autorename = false);
        Object& remove_from_package();
        Package* package() const;
        String full_name() const;
        Counter references() const;
        void add_reference();
        void remove_reference();
        bool is_noname() const;
        String as_string() const;
        Index instance_index() const;
        bool archive_process(Archive& archive) override;
        bool is_valid() const;
        Path filepath() const;
        bool is_editable() const;

        static Package* find_package(const String& name, bool create = false);
        static Package* find_package(const char* name, bool create = false);
        static Package* find_package(const char* name, size_t len, bool create = false);

        virtual Object& preload();
        virtual Object& postload();
        virtual Object& reload();

        Object* owner() const;
        Object& owner(Object* new_owner);

        virtual Object& destroy_script_object(class ScriptObject* object);


        // Override new and delete operators
        static ENGINE_EXPORT void* operator new(size_t size) noexcept;
        static ENGINE_EXPORT void operator delete(void* memory, size_t size) noexcept;

        // Deletion controls
        static ENGINE_EXPORT void delete_object(Object* object);
        Object& deferred_destroy();

        // NOTE! You will manually push object to package, if you use this method!
        template<typename Type, typename... Args>
        static Type* new_instance(Args&&... args)
        {
            if constexpr (is_singletone_v<Type>)
            {
                if (Type::instance() != nullptr)
                {
                    return Type::instance();
                }
            }

            return new Type(std::forward<Args>(args)...);
        }

        template<typename Type, typename... Args>
        static Type* new_non_serializable_instance(Args&&... args)
        {
            if constexpr (is_singletone_v<Type>)
            {
                if (Type::instance() != nullptr)
                {
                    return Type::instance();
                }
            }

            Type* instance = new Type(std::forward<Args>(args)...);
            if constexpr (is_object_based_v<Type>)
            {
                instance->flags(IsSerializable, false);
            }
            return instance;
        }

        template<typename Type, typename... Args>
        static Type* new_instance_named(const String& object_name, Args&&... args)
        {
            Type* object = new_instance<Type>(std::forward<Args>(args)...);
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                object->name(object_name, true);
            }
            return object;
        }

        template<typename Type, typename... Args>
        static Type* new_non_serializable_instance_named(const String& object_name, Args&&... args)
        {
            Type* object = new_non_serializable_instance<Type>(std::forward<Args>(args)...);
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                object->name(object_name, true);
            }
            return object;
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

        template<typename Type>
        static Type* instance_cast(Object* object)
        {
            if (!object)
                return nullptr;
            return object->instance_cast<Type>();
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

        virtual ~Object();
        friend class PointerBase;
        friend class Package;
        friend class Archive;
        friend class MemoryManager;
        friend class EngineInstance;
    };


#define declare_class(class_name, base_name)                                                                                     \
protected:                                                                                                                       \
    static class Engine::Class* _M_static_class;                                                                                 \
                                                                                                                                 \
public:                                                                                                                          \
    using This  = class_name;                                                                                                    \
    using Super = base_name;                                                                                                     \
    static Object* static_constructor();                                                                                         \
    static void static_initialize_class();                                                                                       \
    static class Engine::Class* static_class_instance();                                                                         \
    virtual class Engine::Class* class_instance() const override;                                                                \
                                                                                                                                 \
private:

#define implement_initialize_class(name) void name::static_initialize_class()

#define implement_default_initialize_class(name)                                                                                 \
    implement_initialize_class(name)                                                                                             \
    {}

#define implement_class(class_name, namespace_name, flags)                                                                       \
    class Engine::Class* class_name::_M_static_class = nullptr;                                                                  \
    Engine::Object* class_name::static_constructor()                                                                             \
    {                                                                                                                            \
        if constexpr (std::is_abstract_v<class_name>)                                                                            \
        {                                                                                                                        \
            return nullptr;                                                                                                      \
        }                                                                                                                        \
        else                                                                                                                     \
        {                                                                                                                        \
            return Engine::Object::new_instance<class_name>();                                                                   \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    class Engine::Class* class_name::class_instance() const                                                                      \
    {                                                                                                                            \
        return class_name::static_class_instance();                                                                              \
    }                                                                                                                            \
                                                                                                                                 \
    class Engine::Class* class_name::static_class_instance()                                                                     \
    {                                                                                                                            \
        if (!_M_static_class)                                                                                                    \
        {                                                                                                                        \
            constexpr bool has_base_class = !std::is_same_v<class_name, Engine::Object>;                                         \
            _M_static_class               = new Engine::Class(#class_name, #namespace_name, &This::static_constructor,           \
                                                has_base_class ? Super::static_class_instance() : nullptr, flags); \
            _M_static_class->process_type<class_name>();                                                                         \
            Engine::PostInitializeController controller([]() {                                                                   \
                class_name::static_initialize_class();                                                                           \
                class_name::static_class_instance()->post_initialize();                                                          \
            });                                                                                                                  \
        }                                                                                                                        \
        return _M_static_class;                                                                                                  \
    }                                                                                                                            \
    static Engine::InitializeController initialize_##class_name = Engine::InitializeController(                                  \
            []() { class_name::static_class_instance(); }, ENTITY_INITIALIZER_NAME(class_name, namespace_name))


#define implement_class_default_init(class_name, namespace_name)                                                                 \
    implement_default_initialize_class(class_name);                                                                              \
    implement_class(class_name, namespace_name, 0)

#define implement_engine_class(class_name, flags) implement_class(class_name, Engine, flags)
#define implement_engine_class_default_init(class_name) implement_class_default_init(class_name, Engine)
}// namespace Engine
