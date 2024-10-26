#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/name.hpp>

namespace Engine::Refl
{
	enum class FindFlags
	{
		None                   = 0,
		CreateScope            = BIT(0),
		IsRequired             = BIT(2),
		DisableReflectionCheck = BIT(3),
	};

	class ENGINE_EXPORT Object
	{
	private:
		Object* m_owner = nullptr;
		Name m_name;
		String m_name_splitted;

	protected:
		struct ENGINE_EXPORT Link {
			Name class_name;
			const Link* const parent;

			Link(const char* name, const Link* const parent = nullptr);
			bool is_a(const Link* const link) const;
		};

		static const Link* static_link();
		static String concat_scoped_name(StringView scope, StringView name);
		static void accept_next_object(StringView name);

		void full_name(String& out) const;
		void bind_type_id(Identifier type_id);
		void unbind_type_id(Identifier type_id);

		virtual const Link* link() const;
		virtual Object& unregister_subobject(Object* subobject);
		virtual Object& register_subobject(Object* subobject);


		virtual ~Object();

	public:
		using This  = Object;
		using Super = void;

		CallBacks<void(Object*)> on_destroy;

		Object();

		const Name& class_name() const;
		Object& owner(Object* object);
		Object* owner() const;
		const Name& name() const;
		const String& name_splitted() const;
		String full_name() const;
		String scope_name() const;

		virtual Object* find(StringView name, FindFlags flags = FindFlags::None);

		static Object* static_root();
		static Object* static_find(StringView name, FindFlags flags = FindFlags::None);
		static Object* static_find(Identifier type_id);
		static bool destroy_instance(Object* object);

		template<typename T>
		bool is_a() const
			requires(std::is_base_of_v<Object, T>)
		{
			return link()->is_a(T::static_link());
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
			accept_next_object(name);
			return new T(std::forward<Args>(args)...);
		}

		template<typename T>
		static T* static_find(StringView name, FindFlags flags = FindFlags::None)
		{
			return instance_cast<T>(static_find(name, flags));
		}
	};

	declare_enum_operators(FindFlags);

#define declare_reflect_type(name, base)                                                                                         \
protected:                                                                                                                       \
	static const Link* static_link();                                                                                            \
	virtual const Link* link() const override;                                                                                   \
                                                                                                                                 \
public:                                                                                                                          \
	using This  = name;                                                                                                          \
	using Super = base;                                                                                                          \
	friend class Engine::Refl::Object;                                                                                           \
	static name* static_find(StringView object_name, Engine::Refl::FindFlags flags = Engine::Refl::FindFlags::None);             \
	template<typename T>                                                                                                         \
	static T* static_find(StringView object_name, Engine::Refl::FindFlags flags = Engine::Refl::FindFlags::None)                 \
	{                                                                                                                            \
		return Object::static_find<T>(object_name, flags);                                                                       \
	}


#define implement_reflect_type(name)                                                                                             \
	const Engine::Refl::Object::Link* name::static_link()                                                                        \
	{                                                                                                                            \
		static const Engine::Refl::Object::Link link(#name, Super::static_link());                                               \
		return &link;                                                                                                            \
	}                                                                                                                            \
																																 \
	const Engine::Refl::Object::Link* name::link() const                                                                         \
	{                                                                                                                            \
		return name::static_link();                                                                                              \
	}                                                                                                                            \
	name* name::static_find(StringView object_name, Engine::Refl::FindFlags flags)                                               \
	{                                                                                                                            \
		return Engine::Refl::Object::static_find<name>(object_name, flags);                                                      \
	}
}// namespace Engine::Refl
