#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/enums.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/implement.hpp>
#include <Core/serializable_object.hpp>
#include <Core/userdata.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
    class Package;
    class Object;
    using MessageList = List<String>;


    ENGINE_EXPORT const char* operator""_localized(const char* line, size_t len);

    // Head of all classes in the Engine
    class ENGINE_EXPORT Object : public SerializableObject
    {
    public:
        using ObjectClass = Object;

        enum Flag : BitMask
        {
            None             = 0,
            IsSerializable   = BIT(0),
            IsAvailableForGC = BIT(1),
            IsPackage        = BIT(2),
            IsUnreachable    = BIT(3),
            IsEditable       = BIT(4),
            IsDirty          = BIT(5),
        };

    private:
        Package* m_package;
        Object* m_owner;
        Counter m_references;
        Name m_name;
        mutable Index m_instance_index;

    protected:
        static class Class* m_static_class;

    public:
        mutable Flags<Object::Flag> flags;
        UserData userdata;


    private:
        static void create_default_package();
        const Object& remove_from_instances_array() const;
        static void prepare_next_object_allocation();
        bool serialize_object_properties(Archive& ar);

    protected:
        Object();
        bool private_check_instance(const class Class* const check_class) const;
        static Object* noname_object();

        // Override new and delete operators
        static ENGINE_EXPORT void* operator new(size_t size) noexcept;
        static ENGINE_EXPORT void* operator new(size_t size, void*) noexcept;
        static ENGINE_EXPORT void operator delete(void* memory, size_t size) noexcept;

    public:
        using This  = Object;
        using Super = Object;
        static Object* static_constructor();
        static void static_initialize_class();
        static class Class* static_class_instance();
        static HashIndex hash_of_name(const StringView& name);
        virtual class Class* class_instance() const;

        delete_copy_constructors(Object);
        ENGINE_EXPORT static String package_name_of(const StringView& name);
        ENGINE_EXPORT static String object_name_of(const StringView& name);
        ENGINE_EXPORT static StringView package_name_sv_of(const StringView& name);
        ENGINE_EXPORT static StringView object_name_sv_of(const StringView& name);
        ENGINE_EXPORT static const ObjectArray& all_objects();
        ENGINE_EXPORT static Object* find_object(const StringView& object_name);
        ENGINE_EXPORT static Package* root_package();

        ENGINE_EXPORT static const String& language();
        ENGINE_EXPORT static void language(const StringView& new_language);
        ENGINE_EXPORT static const String& localize(const StringView& line);

        const String& string_name() const;
        HashIndex hash_index() const;
        ObjectRenameStatus name(StringView name, bool autorename = false);
        const Name& name() const;
        virtual Object* copy();
        bool add_to_package(Package* package, bool autorename = false);
        Object& remove_from_package();
        Package* package() const;
        String full_name(bool override_by_owner = false) const;
        Counter references() const;
        void add_reference();
        void remove_reference();
        bool is_noname() const;
        String as_string() const;
        Index instance_index() const;
        bool archive_process(Archive& archive) override;
        Path filepath() const;
        bool is_editable() const;
        bool is_serializable() const;
        virtual bool is_valid() const;
        virtual bool is_engine_resource() const;
        virtual const Object& mark_dirty() const;
        bool is_dirty() const;

        virtual bool save(class BufferWriter* writer = nullptr, Flags<SerializationFlags> flags = {});
        ENGINE_EXPORT static Object* load_object(const StringView& fullname, class BufferReader* reader,
                                                 Flags<SerializationFlags> flags = {});
        ENGINE_EXPORT static Object* load_object(const StringView& fullname, Flags<SerializationFlags> flags = {});
        ENGINE_EXPORT static Object* load_object_from_file(const Path& path, Flags<SerializationFlags> flags = {});

        static Package* find_package(StringView name, bool create = false);

        virtual Object& preload();
        virtual Object& postload();
        virtual Object& apply_changes();

        Object* owner() const;
        Object& owner(Object* new_owner);

        static ENGINE_EXPORT Object* copy_from(Object* src);

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

                return Type::create_instance(std::forward<Args>(args)...);
            }
            else
            {
                return new Type(std::forward<Args>(args)...);
            }
        }

        template<typename Type, typename... Args>
        static Type* new_placement_instance(void* place, Args&&... args)
        {
            if constexpr (is_singletone_v<Type>)
            {
                auto current = Type::instance();

                if (current)
                {
                    return current == place ? current : nullptr;
                }

                return Type::create_placement_instance(place, std::forward<Args>(args)...);
            }
            else
            {
                return new (place) Type(std::forward<Args>(args)...);
            }
        }

        template<typename Type, typename... Args>
        static Type* new_instance_named(const StringView& object_name, Args&&... args)
        {
            Type* object = new_instance<Type>(std::forward<Args>(args)...);
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                if (object)
                    object->name(object_name, true);
            }
            return object;
        }

        template<typename Type, typename... Args>
        static Type* new_placement_instance_named(void* place, const StringView& object_name, Args&&... args)
        {
            Type* object = new_placement_instance<Type>(place, std::forward<Args>(args)...);
            if constexpr (std::is_base_of_v<Object, Type>)
            {
                if (object)
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

        template<typename Type>
        bool leaf_class_is() const
        {
            const void* self = this;
            if (self == nullptr)
                return false;
            return class_instance() == Type::static_class_instance();
        }

        template<typename ObjectInstanceType>
        static ObjectInstanceType* find_object_checked(const StringView& object_name)
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
        friend class GarbageCollector;
    };


#define declare_class(class_name, base_name)                                                                                     \
protected:                                                                                                                       \
    static class Engine::Class* m_static_class;                                                                                  \
                                                                                                                                 \
public:                                                                                                                          \
    using This  = class_name;                                                                                                    \
    using Super = base_name;                                                                                                     \
    static void static_initialize_class();                                                                                       \
    static class Engine::Class* static_class_instance();                                                                         \
    virtual class Engine::Class* class_instance() const override;                                                                \
                                                                                                                                 \
private:

#define implement_class(namespace_name, class_name, flags)                                                                       \
    class Engine::Class* class_name::m_static_class = nullptr;                                                                   \
                                                                                                                                 \
    class Engine::Class* class_name::class_instance() const                                                                      \
    {                                                                                                                            \
        return class_name::static_class_instance();                                                                              \
    }                                                                                                                            \
                                                                                                                                 \
    class Engine::Class* class_name::static_class_instance()                                                                     \
    {                                                                                                                            \
        if (!m_static_class)                                                                                                     \
        {                                                                                                                        \
            constexpr bool has_base_class = !std::is_same_v<class_name, Engine::Object>;                                         \
            m_static_class                = new Engine::Class(ENTITY_INITIALIZER_NAME(class_name, namespace_name),               \
                                               has_base_class ? Super::static_class_instance() : nullptr, flags); \
            m_static_class->setup_class<class_name>();                                                                           \
                                                                                                                                 \
            class_name::static_initialize_class();                                                                               \
            class_name::static_class_instance()->post_initialize();                                                              \
        }                                                                                                                        \
        return m_static_class;                                                                                                   \
    }                                                                                                                            \
    static Engine::ReflectionInitializeController initialize_##class_name = Engine::ReflectionInitializeController(              \
            []() { class_name::static_class_instance(); }, ENTITY_INITIALIZER_NAME(class_name, namespace_name));                 \
    void class_name::static_initialize_class()


#define implement_class_default_init(namespace_name, class_name, flags)                                                          \
    implement_class(namespace_name, class_name, flags)                                                                           \
    {}

#define implement_engine_class(class_name, flags) implement_class(Engine, class_name, flags)
#define implement_engine_class_default_init(class_name, flags) implement_class_default_init(Engine, class_name, flags)
}// namespace Engine
