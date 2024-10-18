#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/templates.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/name.hpp>

namespace Engine
{
	using StructMap = Map<HashIndex, class Struct*>;

	class ENGINE_EXPORT Struct
	{
	public:
		using GroupedPropertiesMap = TreeMap<Name, Vector<class Property*>, Name::Less>;

		struct ENGINE_EXPORT StructCompare {
			bool operator()(const Struct* a, const Struct* b) const
			{
				return a->full_name().to_string() < b->full_name().to_string();
			}
		};

	private:
		void* (*m_alloc)()        = nullptr;
		void (*m_free)(void* mem) = nullptr;

	protected:
		String m_base_name_splitted;

		Name m_full_name;
		Name m_namespace_name;
		Name m_base_name;
		Name m_parent;
		class Group* m_group = nullptr;

		mutable Struct* m_parent_struct = nullptr;
		TreeSet<Struct*, StructCompare> m_childs;
		Vector<class Property*> m_properties;
		GroupedPropertiesMap m_grouped_properties;


		Struct(const Name& ns, const Name& name, const Name& parent = Name::none);
		Struct(const Name& ns, const Name& name, Struct* parent);

		void destroy_childs();

		static ENGINE_EXPORT bool create_internal(const Name& ns, const Name& name, Struct* parent, Struct*& self);

	public:
		template<typename T>
		static Struct* create(const Name& ns, const Name& name)
		{
			Struct* self   = nullptr;
			Struct* parent = nullptr;

			if constexpr (!std::is_same_v<typename T::Super, void>)
			{
				parent = T::Super::static_struct_instance();
			}

			if (create_internal(ns, name, parent, self))
			{
				if constexpr (Concepts::struct_with_custom_allocation<T>)
				{
					self->m_alloc = []() -> void* { return T::static_constructor(); };
					self->m_free  = [](void* mem) { T::static_destructor(reinterpret_cast<T*>(mem)); };
				}
				else
				{
					self->m_alloc = []() -> void* { return new T(); };
					self->m_free  = [](void* mem) { delete reinterpret_cast<T*>(mem); };
				}
			}

			return self;
		}

		static ENGINE_EXPORT Struct* static_find(const StringView& name, bool requred = false);

		const String& name_splitted() const;
		const Name& full_name() const;
		const Name& namespace_name() const;
		const Name& name() const;
		Struct* parent() const;
		virtual void* create_struct() const;
		virtual const Struct& destroy_struct(void* obj) const;
		Struct& group(class Group*);
		class Group* group() const;
		size_t abstraction_level() const;
		Vector<Name> hierarchy(size_t offset = 0) const;
		const TreeSet<Struct*, StructCompare>& child_structs() const;

		bool is_a(const Struct* other) const;
		virtual bool is_class() const;

		Struct& add_property(Property* prop);
		const Vector<class Property*>& properties() const;
		class Property* find_property(const Name& name, bool recursive = false);
		const GroupedPropertiesMap& grouped_properties() const;
		static const StructMap& struct_map();


		template<typename... Args>
		Struct& add_properties(Args&&... args)
		{
			(add_property(std::forward<Args>(args)), ...);
			return *this;
		}
		virtual ~Struct();
	};

#define implement_struct(ns, name)                                                                                               \
	class Engine::Struct* name::m_static_struct = nullptr;                                                                       \
                                                                                                                                 \
	class Engine::Struct* name::static_struct_instance()                                                                         \
	{                                                                                                                            \
		if (!m_static_struct)                                                                                                    \
		{                                                                                                                        \
			m_static_struct = Engine::Struct::create<name>(#ns, #name);                                                          \
			name::static_initialize_struct();                                                                                    \
		}                                                                                                                        \
		return m_static_struct;                                                                                                  \
	}                                                                                                                            \
	static Engine::ReflectionInitializeController initialize_##name =                                                            \
	        Engine::ReflectionInitializeController([]() { name::static_struct_instance(); }, ENTITY_INITIALIZER_NAME(name, ns)); \
	void name::static_initialize_struct()

#define implement_struct_default_init(ns, name)                                                                                  \
	implement_struct(ns, name)                                                                                                   \
	{}
}// namespace Engine
