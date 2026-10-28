#pragma once
#include <Core/enums.hpp>
#include <Core/etl/array.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/types/name.hpp>
#include <angelscript_object.h>

namespace Trinex
{
	class Package;
	class DeprecatedObject;
	class Path;

	trinex_api const char* operator""_localized(const char* line, usize len);

	// Head of all classes in the Engine
	class trinex_api DeprecatedObject : private asIScriptObject
	{
	public:
		template<typename NativeType>
		struct Scriptable : public NativeType
		{
			Scriptable& preload() override
			{
				static_cast<DeprecatedObject*>(this)->script_preload();
				return *this;
			}

			Scriptable& postload() override
			{
				static_cast<DeprecatedObject*>(this)->script_postload();
				return *this;
			}
		};

		using ObjectClass = DeprecatedObject;

		struct Flags
		{
			enum Enum : u32
			{
				None = 0,

				/*  The object is kept around for editing even if is not referenced by anything.
                The object is deleted at the end of the engine operation, or must be deleted manually */
				StandAlone = trinex_bit(0),

				// The object is kept around for editing even if is not referenced by anything. The object must always be deleted manually
				IsAvailableForGC = trinex_bit(1),

				IsSerializable = trinex_bit(2),
				IsUnreachable  = trinex_bit(4),
				IsEditable     = trinex_bit(5),
				IsDirty        = trinex_bit(6),
			};

			trinex_bitfield_enum_struct(Flags, u32);
		};

	private:
		Refl::Deprecated::Class* m_class;
		DeprecatedObject* m_owner;
		Name m_name;

		mutable Atomic<u32> m_references;
		mutable u32 m_child_index;
		mutable u32 m_global_index;

	protected:
		static class Refl::Deprecated::Class* m_static_class;

	public:
		mutable Flags flags;

	private:
		// Setup object info
		static Refl::Deprecated::Class* static_setup_next_object_info(Refl::Deprecated::Class* self = nullptr);
		static void initialize_new_object(DeprecatedObject* object, StringView name, DeprecatedObject* owner);

		template<typename T>
		static inline T* initialize_new_object_checked(T* object, StringView name, DeprecatedObject* owner)
		{
			if constexpr (std::is_base_of_v<DeprecatedObject, T>)
			{
				initialize_new_object(object, name, owner);
			}
			return object;
		}

		template<u32 DeprecatedObject::* prop = &DeprecatedObject::m_child_index, typename ContainerType>
		bool remove_from(ContainerType& container, u32 idx)
		{
			if (container.empty())
				return false;

			const u32 last = container.size() - 1;

			if (idx >= container.size() || container[idx] != this)
				return false;

			if (idx != last)
			{
				auto* moved                                  = container[last];
				container[idx]                               = moved;
				static_cast<DeprecatedObject*>(moved)->*prop = idx;
			}

			container.pop_back();
			return true;
		}

		void script_preload();
		void script_postload();

	protected:
		bool private_check_instance(const class Refl::Deprecated::Class* const check_class) const;

		virtual DeprecatedObject& on_owner_update(DeprecatedObject* new_owner);
		virtual DeprecatedObject* register_child(DeprecatedObject* child, u32& index);
		virtual bool unregister_child(DeprecatedObject* child);
		virtual DeprecatedObject& post_rename(DeprecatedObject* old_owner, Name old_name);

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
		using This  = DeprecatedObject;
		using Super = DeprecatedObject;

		DeprecatedObject();
		trinex_non_copyable(DeprecatedObject);
		trinex_non_moveable(DeprecatedObject);

		static DeprecatedObject* static_constructor();
		static void static_initialize_class();
		static class Refl::Deprecated::Class* static_reflection();
		class Refl::Deprecated::Class* class_instance() const;

		static String package_name_of(const StringView& name);
		static String object_name_of(const StringView& name);
		static StringView package_name_sv_of(const StringView& name);
		static StringView object_name_sv_of(const StringView& name);
		static const Array<DeprecatedObject*>& static_root_objects();
		static const Array<DeprecatedObject*>& static_objects();
		static Package* root_package();
		static bool static_validate_object_name(StringView name, String* msg = nullptr);

		static const String& language();
		static void language(const StringView& new_language);
		static const String& localize(const StringView& line);

		static DeprecatedObject* load_object(StringView fullname, class Blob* blob, ArchiveFlags flags = {});
		static DeprecatedObject* load_object(StringView fullname, class Stream* stream, ArchiveFlags flags = {});
		static DeprecatedObject* load_object(StringView fullname, ArchiveFlags flags = {});
		static DeprecatedObject* load_object_from_file(const Path& path, ArchiveFlags flags = {});

		virtual bool rename(StringView name, DeprecatedObject* new_owner = nullptr);
		const Name& name() const;

		static DeprecatedObject* static_find_object(StringView object_name);
		static Package* static_find_package(StringView name, bool create = false);
		virtual DeprecatedObject* find_child_object(StringView name) const;

		const String& string_name() const;
		Package* package(bool recursive = false) const;
		String full_name() const;
		u32 references() const;
		u32 add_reference() const;
		u32 remove_reference() const;
		bool is_noname() const;
		u32 child_index() const;
		u32 global_index() const;
		virtual bool serialize(Archive& archive);
		Path filepath() const;
		bool is_editable() const;
		bool is_serializable() const;
		virtual bool is_valid() const;
		virtual const DeprecatedObject& mark_dirty() const;
		bool is_dirty() const;

		virtual bool save(Stream* stream = nullptr, ArchiveFlags flags = {});

		virtual DeprecatedObject& on_create();
		virtual DeprecatedObject& on_destroy();
		virtual DeprecatedObject& on_property_changed(const Refl::Deprecated::PropertyChangedEvent& event);

		virtual DeprecatedObject& preload();
		virtual DeprecatedObject& postload();
		virtual DeprecatedObject& rebuild();

		DeprecatedObject* owner() const;
		bool owner(DeprecatedObject* new_owner);

		static trinex_api DeprecatedObject* copy_from(DeprecatedObject* src);

		static DeprecatedObject* static_new_instance(Refl::Deprecated::Class* object_class, StringView name = "",
		                                             DeprecatedObject* owner = nullptr);
		static DeprecatedObject* static_new_placement_instance(void* place, Refl::Deprecated::Class* object_class, StringView name = "",
		                                                       DeprecatedObject* owner = nullptr);

		template<typename Type, bool check_constructible = false, typename... Args>
		static Type* new_instance(StringView name = "", DeprecatedObject* owner = nullptr, Args&&... args)
		{
			constexpr bool invalid =
			        check_constructible && (std::is_abstract_v<Type> || (!std::is_constructible_v<Type, Args...>) );

			if constexpr (invalid)
			{
				return nullptr;
			}
			else
			{
				if constexpr (std::is_base_of_v<DeprecatedObject, Type>)
					static_setup_next_object_info(Type::static_reflection());
				return initialize_new_object_checked(trx_new Type(std::forward<Args>(args)...), name, owner);
			}
		}

		template<typename Type, bool check_constructible = false, typename... Args>
		static Type* new_placement_instance(void* place, StringView name = "", DeprecatedObject* owner = nullptr, Args&&... args)
		{
			constexpr bool invalid =
			        check_constructible && (std::is_abstract_v<Type> || (!std::is_constructible_v<Type, Args...>) );

			if constexpr (invalid)
			{
				return nullptr;
			}
			else
			{
				if constexpr (std::is_base_of_v<DeprecatedObject, Type>)
					static_setup_next_object_info(Type::static_reflection());
				return initialize_new_object_checked(new (place) Type(std::forward<Args>(args)...), name, owner);
			}
		}

		template<typename Type>
		typename std::enable_if<etl::is_object_type<Type>::value, bool>::type is_instance_of() const
		{
			return private_check_instance(Type::static_reflection());
		}

		template<typename Type>
		typename std::enable_if<!etl::is_object_type<Type>::value, bool>::type is_instance_of() const
		{
			return false;
		}

		template<typename Type>
		typename std::enable_if<etl::is_object_type<Type>::value, const Type*>::type instance_cast() const
		{
			if (!is_instance_of<Type>())
				return nullptr;
			return (const Type*) this;
		}

		template<typename Type>
		typename std::enable_if<etl::is_object_type<Type>::value, Type*>::type instance_cast()
		{
			if (!is_instance_of<Type>())
				return nullptr;
			return (Type*) this;
		}

		template<typename Type>
		typename std::enable_if<!etl::is_object_type<Type>::value, const Type*>::type instance_cast() const
		{
			return nullptr;
		}

		template<typename Type>
		typename std::enable_if<!etl::is_object_type<Type>::value, Type*>::type instance_cast()
		{
			return nullptr;
		}

		template<typename Type>
		static Type* instance_cast(DeprecatedObject* object)
		{
			if (!object)
				return nullptr;
			return object->instance_cast<Type>();
		}

		template<typename Type>
		static const Type* instance_cast(const DeprecatedObject* object)
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
			return class_instance() == Type::static_reflection();
		}

		template<typename ObjectInstanceType>
		static ObjectInstanceType* static_find_object_checked(const StringView& object_name)
		{
			DeprecatedObject* object = static_find_object(object_name);
			if (object)
			{
				return object->instance_cast<ObjectInstanceType>();
			}
			return nullptr;
		}

		template<typename ObjectInstanceType>
		ObjectInstanceType* find_child_object_checked(StringView object_name) const
		{
			DeprecatedObject* object = find_child_object(object_name);
			if (object)
			{
				return instance_cast<ObjectInstanceType>(object);
			}
			return nullptr;
		}

		template<typename ContainerType>
		bool remove_from(ContainerType& container)
		{
			if (remove_from(container, m_child_index))
			{
				m_child_index = 0xFFFFFFFF;
				return true;
			}

			return false;
		}

		virtual ~DeprecatedObject();
		friend class PointerBase;
		friend class Package;
		friend class Archive;
		friend class MemoryManager;
		friend class GarbageCollector;
		friend class Refl::Deprecated::Class;
	};

#define trinex_class(class_name, base_name)                                                                                      \
protected:                                                                                                                       \
	static class Trinex::Refl::Deprecated::Class* m_static_class;                                                                            \
                                                                                                                                 \
public:                                                                                                                          \
	using This  = class_name;                                                                                                    \
	using Super = base_name;                                                                                                     \
	static void static_initialize_class();                                                                                       \
	static class Trinex::Refl::Deprecated::Class* static_reflection();                                                                       \
                                                                                                                                 \
private:

#define trinex_implement_class(decl, flags)                                                                                      \
	class Trinex::Refl::Deprecated::Class* decl::m_static_class = nullptr;                                                                   \
                                                                                                                                 \
	class Trinex::Refl::Deprecated::Class* decl::static_reflection()                                                                         \
	{                                                                                                                            \
		if (!m_static_class)                                                                                                     \
		{                                                                                                                        \
			m_static_class = Trinex::Refl::Deprecated::NativeClass<decl>::create(#decl, flags);                                              \
		}                                                                                                                        \
		return m_static_class;                                                                                                   \
	}                                                                                                                            \
	trinex_on_reflection_init(.name = #decl)                                                                                     \
	{                                                                                                                            \
		decl::static_reflection();                                                                                               \
	}                                                                                                                            \
	void decl::static_initialize_class()

#define trinex_implement_class_default_init(decl, flags)                                                                         \
	trinex_implement_class(decl, flags) {}

#define trinex_implement_engine_class(decl, flags) trinex_implement_class(Trinex::decl, flags)
#define trinex_implement_engine_class_default_init(decl, flags)                                                                  \
	trinex_implement_engine_class(decl, flags) {}
} // namespace Trinex
