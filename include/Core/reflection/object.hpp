#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/any.hpp>
#include <Core/etl/map.hpp>
#include <Core/name.hpp>

namespace Engine
{
	class ScriptClassRegistrar;
}

namespace Engine::Refl
{
	namespace Meta
	{
		extern ENGINE_EXPORT Name display_name;
		extern ENGINE_EXPORT Name tooltip;
		extern ENGINE_EXPORT Name description;
		extern ENGINE_EXPORT Name group;
		extern ENGINE_EXPORT Name renderer;
	}// namespace Meta

	enum class FindFlags
	{
		None                   = 0,
		CreateScope            = BIT(0),
		IsRequired             = BIT(2),
		DisableReflectionCheck = BIT(3),
	};

	struct ENGINE_EXPORT ClassInfo {
		const Name class_name;
		const ClassInfo* const parent;
		bool is_scriptable;

		ClassInfo(const char* name, const ClassInfo* const parent = nullptr);
		bool is_a(const ClassInfo* const info) const;
	};

	class ENGINE_EXPORT Object
	{
	private:
		using MetaData       = Map<Name, Any, Name::HashFunction>;
		MetaData* m_metadata = nullptr;
		Object* m_owner      = nullptr;

		Name m_name;
		bool m_is_initialized : 1 = false;

		void setup_owner();

	protected:
		static String concat_scoped_name(StringView scope, StringView name);
		static void initialize_next_object(StringView name);
		static void initialize_next_object(Object* owner, StringView name);

		void full_name(String& out) const;

		FORCE_INLINE bool has_metadata(const Name& name) const
		{
			return find_metadata(name) != nullptr;
		}

		virtual Object& unregister_subobject(Object* subobject);
		virtual Object& register_subobject(Object* subobject);
		virtual Object& initialize();
		virtual Object& construct();

		virtual ~Object();

	public:
		using This     = Object;
		using Super    = void;
		using DownCast = void* (*) (Object*);

		CallBacks<void(Object*)> on_initialize;

		Object();
		Object(const Object&) = delete;
		Object& owner(Object* object);
		Object* owner() const;
		const Name& name() const;
		String full_name() const;
		String scope_name() const;
		bool is_initialized() const;

		const String& display_name() const;
		const String& tooltip() const;
		const String& description() const;
		const String& group() const;

		Object& display_name(StringView name);
		Object& tooltip(StringView text);
		Object& description(StringView text);
		Object& group(StringView text);

		const Any* find_metadata(const Name& name) const;
		const Any& metadata(const Name& name) const;
		Object& metadata(const Name& name, const Any& any);
		Object& remove_metadata(const Name& name);

		virtual Object* find(StringView name, FindFlags flags = FindFlags::None);
		virtual ClassInfo* refl_class_info() const;

		static ClassInfo* static_refl_class_info();
		static Object* static_root();
		static Object* static_find(StringView name, FindFlags flags = FindFlags::None);
		static Object* static_require(StringView name, FindFlags flags = FindFlags::None);
		static void static_initialize(Object* root = nullptr, bool force_recursive = false);
		static bool destroy_instance(Object* object);
		static bool is_valid(Object* object);
		static void register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast);

		template<typename T>
		bool is_a() const
			requires(std::is_base_of_v<Object, T>)
		{
			return refl_class_info()->is_a(T::static_refl_class_info());
		}

		template<typename T, typename ObjectType>
		static T* instance_cast(ObjectType* object)
			requires(std::is_base_of_v<Object, T> && std::is_base_of_v<Object, ObjectType>)
		{
			if (object == nullptr)
				return nullptr;

			if constexpr (std::is_base_of_v<T, ObjectType>)
			{
				return object;
			}
			else
			{
				if (static_cast<const Object*>(object)->is_a<T>())
				{
					return reinterpret_cast<T*>(object);
				}
				return nullptr;
			}
		}

		template<typename T, typename... Args>
		static T* new_instance(StringView name, Args&&... args)
			requires(std::is_base_of_v<Object, T>)
		{
			initialize_next_object(name);
			T* instance = new T(std::forward<Args>(args)...);
			static_cast<Object*>(instance)->setup_owner();
			static_cast<Object*>(instance)->construct();
			return instance;
		}

		template<typename T, typename... Args>
		static T* new_instance(Object* owner, StringView name, Args&&... args)
			requires(std::is_base_of_v<Object, T>)
		{
			initialize_next_object(owner, name);
			T* instance = new T(std::forward<Args>(args)...);
			static_cast<Object*>(instance)->setup_owner();
			static_cast<Object*>(instance)->construct();
			return instance;
		}

		template<typename T, typename... Args>
		static T* new_instance(Object* owner, const char* name, Args&&... args)
			requires(std::is_base_of_v<Object, T>)
		{
			return new_instance<T>(owner, StringView(name), std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		T* new_child(StringView name, Args&&... args)
			requires(std::is_base_of_v<Object, T>)
		{
			return new_instance<T>(this, name, std::forward<Args>(args)...);
		}

		template<typename T>
		static T* static_find(StringView name, FindFlags flags = FindFlags::None)
		{
			return instance_cast<T>(static_find(name, flags));
		}

		template<typename T>
		T* find(StringView name, FindFlags flags = FindFlags::None)
		{
			return instance_cast<T>(find(name, flags));
		}

		template<typename T>
		static T* static_require(StringView name, FindFlags flags = FindFlags::None)
		{
			return instance_cast<T>(static_require(name, flags));
		}

		template<typename Type>
		bool leaf_is() const
			requires(std::is_base_of_v<Object, Type>)
		{
			const void* self = this;
			if (self == nullptr)
				return false;
			return refl_class_info() == Type::static_refl_class_info();
		}
	};

	template<typename T>
	void* script_downcast(Object* object)
	{
		return Object::instance_cast<T>(object);
	}

	declare_enum_operators(FindFlags);

#define declare_reflect_type(name, base)                                                                                         \
public:                                                                                                                          \
	using This  = name;                                                                                                          \
	using Super = base;                                                                                                          \
	friend class Engine::Refl::Object;                                                                                           \
																																 \
	static Engine::Refl::ClassInfo* static_refl_class_info();                                                                    \
	virtual Engine::Refl::ClassInfo* refl_class_info() const override;                                                           \
																																 \
	static name* static_find(StringView object_name, Engine::Refl::FindFlags flags = Engine::Refl::FindFlags::None);             \
	static name* static_require(StringView object_name, Engine::Refl::FindFlags flags = Engine::Refl::FindFlags::None);          \
	template<typename T>                                                                                                         \
	static T* static_find(StringView object_name, Engine::Refl::FindFlags flags = Engine::Refl::FindFlags::None)                 \
	{                                                                                                                            \
		return Object::static_find<T>(object_name, flags);                                                                       \
	}                                                                                                                            \
	template<typename T>                                                                                                         \
	static T* static_require(StringView object_name, Engine::Refl::FindFlags flags = Engine::Refl::FindFlags::None)              \
	{                                                                                                                            \
		return Object::static_require<T>(object_name, flags);                                                                    \
	}


#define implement_reflect_type(name)                                                                                             \
	Engine::Refl::ClassInfo* name::static_refl_class_info()                                                                      \
	{                                                                                                                            \
		static Engine::Refl::ClassInfo info(#name, Super::static_refl_class_info());                                             \
		return &info;                                                                                                            \
	}                                                                                                                            \
																																 \
	Engine::Refl::ClassInfo* name::refl_class_info() const                                                                       \
	{                                                                                                                            \
		return name::static_refl_class_info();                                                                                   \
	}                                                                                                                            \
	name* name::static_find(StringView object_name, Engine::Refl::FindFlags flags)                                               \
	{                                                                                                                            \
		return Engine::Refl::Object::static_find<name>(object_name, flags);                                                      \
	}                                                                                                                            \
	name* name::static_require(StringView object_name, Engine::Refl::FindFlags flags)                                            \
	{                                                                                                                            \
		return Engine::Refl::Object::static_require<name>(object_name, flags);                                                   \
	}
}// namespace Engine::Refl
