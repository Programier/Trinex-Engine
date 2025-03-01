#pragma once
#include <Core/engine_types.hpp>
#include <Core/enums.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/etl/vector.hpp>
#include <Core/flags.hpp>
#include <Core/name.hpp>
#include <angelscript_object.h>

namespace Engine
{
	class Package;
	class Object;
	class Path;

	ENGINE_EXPORT const char* operator""_localized(const char* line, size_t len);

	// Head of all classes in the Engine
	class ENGINE_EXPORT Object : private asIScriptObject
	{
	public:
		template<typename NativeType>
		struct Scriptable : public NativeType {
			Scriptable& preload() override
			{
				static_cast<Object*>(this)->script_preload();
				return *this;
			}

			Scriptable& postload() override
			{
				static_cast<Object*>(this)->script_postload();
				return *this;
			}
		};


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
			IsScriptable   = BIT(7),
		};

	private:
		Refl::Class* m_class;
		Object* m_owner;
		mutable Atomic<Counter> m_references;
		Name m_name;
		mutable Index m_instance_index;

	protected:
		static class Refl::Class* m_static_class;

	public:
		mutable Flags<Object::Flag> flags;

	private:
		// Setup object info
		static Refl::Class* setup_next_object_info(Refl::Class* self);
		static void reset_next_object_info();

		static void create_default_package();
		const Object& remove_from_instances_array() const;

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

		void script_preload();
		void script_postload();

	protected:
		bool private_check_instance(const class Refl::Class* const check_class) const;
		static Object* noname_object();

		virtual Object& on_owner_update(Object* new_owner);
		virtual bool register_child(Object* child);
		virtual bool unregister_child(Object* child);
		virtual Object& post_rename(Object* old_owner, Name old_name);

		// Override new and delete operators
		static ENGINE_EXPORT void* operator new(size_t size) noexcept;
		static ENGINE_EXPORT void* operator new(size_t size, void*) noexcept;
		static ENGINE_EXPORT void operator delete(void* memory, size_t size) noexcept;

	private:
		// AngelScript integration
		int AddRef() const override;
		int Release() const override;
		asILockableSharedBool* GetWeakRefFlag() const override;
		int GetRefCount() override;
		void SetGCFlag() override;
		bool GetGCFlag() override;
		asITypeInfo* GetObjectType() const override;
		int CopyFrom(const asIScriptObject* other) override;

	public:
		using This  = Object;
		using Super = Object;

		Object();

		static Object* static_constructor();
		static void static_initialize_class();
		static class Refl::Class* static_class_instance();
		static HashIndex hash_of_name(const StringView& name);
		class Refl::Class* class_instance() const;

		delete_copy_constructors(Object);
		static String package_name_of(const StringView& name);
		static String object_name_of(const StringView& name);
		static StringView package_name_sv_of(const StringView& name);
		static StringView object_name_sv_of(const StringView& name);
		static const Vector<Object*>& all_objects();
		static Package* root_package();
		static bool static_validate_object_name(StringView name, String* msg = nullptr);

		static const String& language();
		static void language(const StringView& new_language);
		static const String& localize(const StringView& line);

		virtual bool rename(StringView name, Object* new_owner = nullptr);
		const Name& name() const;

		static Object* static_find_object(StringView object_name);
		static Package* static_find_package(StringView name, bool create = false);
		virtual Object* find_child_object(StringView name) const;

		const String& string_name() const;
		HashIndex hash_index() const;
		Package* package(bool recursive = false) const;
		String full_name() const;
		Counter references() const;
		size_t add_reference() const;
		size_t remove_reference() const;
		bool is_noname() const;
		Index instance_index() const;
		virtual bool serialize(Archive& archive);
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
		virtual Object& begin_destroy();
		virtual Object& on_property_changed(const Refl::PropertyChangedEvent& event);

		Object* owner() const;
		bool owner(Object* new_owner);

		static ENGINE_EXPORT Object* copy_from(Object* src);

		static Object* static_new_instance(Refl::Class* object_class, StringView name = "", Object* owner = nullptr);
		static Object* static_new_placement_instance(void* place, Refl::Class* object_class, StringView name = "",
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
		ObjectInstanceType* find_child_object_checked(StringView object_name) const
		{
			Object* object = find_child_object(object_name);
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
		friend class Refl::Class;
	};


#define declare_class(class_name, base_name)                                                                                     \
protected:                                                                                                                       \
    static class Engine::Refl::Class* m_static_class;                                                                            \
                                                                                                                                 \
public:                                                                                                                          \
	using This  = class_name;                                                                                                    \
	using Super = base_name;                                                                                                     \
	static void static_initialize_class();                                                                                       \
	static class Engine::Refl::Class* static_class_instance();                                                                   \
                                                                                                                                 \
private:

#define implement_class(decl, flags)                                                                                             \
    class Engine::Refl::Class* decl::m_static_class = nullptr;                                                                   \
                                                                                                                                 \
    class Engine::Refl::Class* decl::static_class_instance()                                                                     \
    {                                                                                                                            \
        if (!m_static_class)                                                                                                     \
        {                                                                                                                        \
            m_static_class = Engine::Refl::NativeClass<decl>::create(#decl, flags);                                              \
        }                                                                                                                        \
        return m_static_class;                                                                                                   \
    }                                                                                                                            \
    static Engine::byte TRINEX_CONCAT(trinex_engine_refl_class_, __LINE__) = static_cast<Engine::byte>(                          \
            Engine::Refl::Object::static_register_initializer([]() { decl::static_class_instance(); }, #decl));                  \
    void decl::static_initialize_class()


#define implement_class_default_init(decl, flags)                                                                                \
	implement_class(decl, flags)                                                                                                 \
	{}

#define implement_engine_class(decl, flags) implement_class(Engine::decl, flags)
#define implement_engine_class_default_init(decl, flags)                                                                         \
	implement_engine_class(decl, flags)                                                                                          \
	{}
}// namespace Engine
