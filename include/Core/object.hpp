#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/enums.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/implement.hpp>
#include <Core/name.hpp>
#include <Core/serializable_object.hpp>
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
            None = 0,

            /*  The object is kept around for editing even if is not referenced by anything.
                The object is deleted at the end of the engine operation, or must be deleted manually */
            StandAlone = BIT(0),

            // The object is kept around for editing even if is not referenced by anything. The object must always be deleted manually
            IsAvailableForGC = BIT(1),

            IsSerializable = BIT(2),
            IsPackage      = BIT(3),
            IsUnreachable  = BIT(4),
            IsEditable     = BIT(5),
            IsDirty        = BIT(6),
        };

    private:
        Class* m_class;
        Object* m_owner;
        mutable Counter m_references;
        Name m_name;
        mutable Index m_instance_index;

    protected:
        static class Class* m_static_class;

    public:
        mutable Flags<Object::Flag> flags;

    private:
        // Setup object info
        static void setup_next_object_info(Class* self);
        static void reset_next_object_info();

        static void create_default_package();
        const Object& remove_from_instances_array() const;
        bool serialize_object_properties(Archive& ar);

        template<typename T>
        static FORCE_INLINE T* setup_new_object(T* object, StringView name, Object* owner)
        {
            if constexpr (std::is_base_of_v<Object, T>)
            {
                Object* base_object = object;
                base_object->m_name = name;
                base_object->owner(owner);
                reset_next_object_info();
            }
            return object;
        }

    protected:
        Object();
        bool private_check_instance(const class Class* const check_class) const;
        static Object* noname_object();

        virtual Object& on_owner_update(Object* new_owner);
        virtual bool register_child(Object* child);
        virtual bool unregister_child(Object* child);
        virtual bool rename_child_object(Object* object, StringView new_name);
        virtual Object& post_rename(Object* old_owner, Name old_name);

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
        class Class* class_instance() const;

        delete_copy_constructors(Object);
        static String package_name_of(const StringView& name);
        static String object_name_of(const StringView& name);
        static StringView package_name_sv_of(const StringView& name);
        static StringView object_name_sv_of(const StringView& name);
        static const ObjectArray& all_objects();
        static Package* root_package();

        static const String& language();
        static void language(const StringView& new_language);
        static const String& localize(const StringView& line);

        virtual bool rename(StringView name, Object* new_owner = nullptr);
        const Name& name() const;

        static Object* static_find_object(StringView object_name);
        static Package* static_find_package(StringView name, bool create = false);
        virtual Object* find_child_object(StringView name, bool recursive = true) const;

        const String& string_name() const;
        HashIndex hash_index() const;
        Package* package(bool recursive = false) const;
        String full_name(bool override_by_owner = false) const;
        Counter references() const;
        size_t add_reference() const;
        size_t remove_reference() const;
        bool is_noname() const;
        String as_string() const;
        Index instance_index() const;
        bool archive_process(Archive& archive) override;
        Path filepath() const;
        bool is_editable() const;
        bool is_serializable() const;
        virtual bool is_valid() const;
        virtual const Object& mark_dirty() const;
        bool is_dirty() const;

        virtual bool save(class BufferWriter* writer = nullptr, Flags<SerializationFlags> flags = {});
        ENGINE_EXPORT static Object* load_object(StringView fullname, class BufferReader* reader,
                                                 Flags<SerializationFlags> flags = {});
        ENGINE_EXPORT static Object* load_object(StringView fullname, Flags<SerializationFlags> flags = {});
        ENGINE_EXPORT static Object* load_object_from_file(const Path& path, Flags<SerializationFlags> flags = {});


        virtual Object& preload();
        virtual Object& postload();
        virtual Object& apply_changes();

        Object* owner() const;
        bool owner(Object* new_owner);

        static ENGINE_EXPORT Object* copy_from(Object* src);

        static Object* static_new_instance(Class* object_class, StringView name = "", Object* owner = nullptr);
        static Object* static_new_placement_instance(void* place, Class* object_class, StringView name = "",
                                                     Object* owner = nullptr);

        template<typename Type, bool check_constructible = false, typename... Args>
        static Type* new_instance(StringView name = "", Object* owner = nullptr, Args&&... args)
        {
            constexpr bool invalid =
                    check_constructible &&
                    (std::is_abstract_v<Type> || (!std::is_constructible_v<Type, Args...> && !Engine::is_singletone_v<Type>) );

            if constexpr (invalid)
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

                    if constexpr (std::is_base_of_v<Object, Type>)
                        setup_next_object_info(Type::static_class_instance());
                    return setup_new_object(Type::create_instance(std::forward<Args>(args)...), name, owner);
                }
                else
                {
                    if constexpr (std::is_base_of_v<Object, Type>)
                        setup_next_object_info(Type::static_class_instance());
                    return setup_new_object(new Type(std::forward<Args>(args)...), name, owner);
                }
            }
        }

        template<typename Type, bool check_constructible = false, typename... Args>
        static Type* new_placement_instance(void* place, StringView name = "", Object* owner = nullptr, Args&&... args)
        {
            constexpr bool invalid =
                    check_constructible &&
                    (std::is_abstract_v<Type> || (!std::is_constructible_v<Type, Args...> && !Engine::is_singletone_v<Type>) );

            if constexpr (invalid)
            {
                return nullptr;
            }
            else
            {
                if constexpr (is_singletone_v<Type>)
                {
                    auto current = Type::instance();

                    if (current)
                    {
                        return current == place ? current : nullptr;
                    }

                    if constexpr (std::is_base_of_v<Object, Type>)
                        setup_next_object_info(Type::static_class_instance());
                    return setup_new_object(Type::create_placement_instance(place, std::forward<Args>(args)...), name, owner);
                }
                else
                {
                    if constexpr (std::is_base_of_v<Object, Type>)
                        setup_next_object_info(Type::static_class_instance());
                    return setup_new_object(new (place) Type(std::forward<Args>(args)...), name, owner);
                }
            }
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
        static ObjectInstanceType* static_find_object_checked(const StringView& object_name)
        {
            Object* object = static_find_object(object_name);
            if (object)
            {
                return object->instance_cast<ObjectInstanceType>();
            }
            return nullptr;
        }

        template<typename ObjectInstanceType>
        ObjectInstanceType* find_child_object_checked(StringView object_name, bool recursive = true) const
        {
            Object* object = find_child_object(object_name, recursive);
            if (object)
            {
                return instance_cast<ObjectInstanceType>(object);
            }
            return nullptr;
        }


        virtual ~Object();
        friend class PointerBase;
        friend class Package;
        friend class Archive;
        friend class MemoryManager;
        friend class GarbageCollector;
        friend class Class;
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
    friend class Engine::Class;                                                                                                  \
                                                                                                                                 \
private:

#define implement_class(namespace_name, class_name, flags)                                                                       \
    class Engine::Class* class_name::m_static_class = nullptr;                                                                   \
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
