#pragma once
#include <Core/etl/type_info.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/reflection/scoped_type.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine
{
	class Property;
	class Group;
}// namespace Engine

namespace Engine::Refl
{
	class ENGINE_EXPORT Struct : public ScopedType
	{
		declare_reflect_type(Struct, ScopedType);

	public:
		struct ENGINE_EXPORT StructCompare {
			bool operator()(const Struct* a, const Struct* b) const
			{
				return a->name().to_string() < b->name().to_string();
			}
		};

		enum Flag : BitMask
		{
			IsSingletone    = BIT(0),
			IsAbstract      = BIT(1),
			IsConstructible = BIT(2),
			IsFinal         = BIT(3),
			IsNative        = BIT(4),
			IsScriptable    = BIT(5),
			IsAsset         = BIT(6),
		};

		Flags<Flag> flags;
		ScriptTypeInfo script_type_info;

	private:
		Vector<Property*> m_properties;
		Set<Struct*> m_childs;
		mutable Struct* m_parent = nullptr;

		class Group* m_group      = nullptr;
		void* (*m_alloc)()        = nullptr;
		void (*m_free)(void* mem) = nullptr;
		StringView m_type_name;

		template<typename T>
		inline static Struct* super_of()
			requires(has_super_type_v<T> && !std::is_same_v<typename T::Super, void>)
		{
			return T::Super::static_struct_instance();
		}

		template<typename T>
		inline static Struct* super_of()
		{
			return nullptr;
		}

	protected:
		void destroy_childs();

	public:
		Struct(Struct* parent = nullptr, BitMask flags = 0, StringView type_name = "");

		template<typename T>
		static Struct* create(StringView decl)
		{
			Struct* parent = super_of<T>();

			if (Struct* self = Object::new_instance<Struct>(decl, parent, 0, type_info<T>::name()))
			{
				if constexpr (Concepts::struct_with_custom_allocation<T>)
				{
					if (self->m_alloc == nullptr)
						self->m_alloc = []() -> void* { return T::static_constructor(); };
					if (self->m_free == nullptr)
						self->m_free = [](void* mem) { T::static_destructor(reinterpret_cast<T*>(mem)); };
				}
				else
				{
					if (self->m_alloc == nullptr)
						self->m_alloc = []() -> void* { return new T(); };
					if (self->m_free == nullptr)
						self->m_free = [](void* mem) { delete reinterpret_cast<T*>(mem); };
				}
				return self;
			}

			return nullptr;
		}

		virtual void* create_struct() const;
		virtual const Struct& destroy_struct(void* obj) const;

		Struct* parent() const;
		size_t abstraction_level() const;
		Vector<Name> hierarchy(size_t offset = 0) const;
		const Set<Struct*>& childs() const;

		using Super::is_a;
		bool is_a(const Struct* other) const;
		bool is_class() const;

		Struct& add_property(Property* prop);
		const Vector<Property*>& properties() const;
		Property* find_property(const Name& name, bool recursive = false);

		Struct& group(class Group*);
		class Group* group() const;

		FORCE_INLINE StringView type_name() const
		{
			return m_type_name;
		}

		template<typename... Args>
		Struct& add_properties(Args&&... args)
		{
			(add_property(std::forward<Args>(args)), ...);
			return *this;
		}

		~Struct();
	};

#define implement_struct(decl)                                                                                                   \
    class Engine::Refl::Struct* decl::m_static_struct = nullptr;                                                                 \
                                                                                                                                 \
    class Engine::Refl::Struct* decl::static_struct_instance()                                                                   \
    {                                                                                                                            \
        if (!m_static_struct)                                                                                                    \
        {                                                                                                                        \
            m_static_struct = Engine::Refl::Struct::create<decl>(#decl);                                                         \
            decl::static_initialize_struct();                                                                                    \
        }                                                                                                                        \
        return m_static_struct;                                                                                                  \
    }                                                                                                                            \
                                                                                                                                 \
    static Engine::byte TRINEX_CONCAT(trinex_engine_refl_struct_, __LINE__) = static_cast<Engine::byte>(                         \
            Engine::ReflectionInitializeController([]() { decl::static_struct_instance(); }, #decl).id());                       \
                                                                                                                                 \
    void decl::static_initialize_struct()

#define implement_struct_default_init(decl)                                                                                      \
	implement_struct(decl)                                                                                                       \
	{}
}// namespace Engine::Refl
