#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/name.hpp>
#include <Core/object.hpp>
#include <Core/struct.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine
{
	class ScriptClassRegistrar;

	class ENGINE_EXPORT Class final : public Struct
	{
	public:
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

		Flags<Class::Flag> flags;
		using ChildsSet = TreeSet<Class*, StructCompare>;

	private:
		mutable Object* m_singletone_object;

		void (*m_destroy_func)(Object*)                                               = nullptr;
		Object* (*m_script_factory)(StringView, Object*)                              = nullptr;
		Object* (*m_static_constructor)(Class*, StringView, Object*)                  = nullptr;
		Object* (*m_static_placement_constructor)(Class*, void*, StringView, Object*) = nullptr;
		ChildsSet m_childs;
		size_t m_size;

		void on_create_call(Object* object) const;
		void bind_class_to_script_engine();
		void register_scriptable_class();

	public:
		ScriptTypeInfo script_type_info;
		CallBacks<void(Object*)> on_create;
		CallBacks<void(Object*)> on_destroy;
		CallBacks<void(Class*)> on_class_destroy;
		Function<void(ScriptClassRegistrar*, Class*)> script_registration_callback;

		Class(const Name& full_name, Class* parent = nullptr, BitMask flags = 0);

		Class* parent() const;
		const ChildsSet& child_classes() const;

		void (*destroy_func() const)(Object*);
		Class& destroy_func(void (*)(Object*));

		void* create_struct() const override;
		Object* create_object(StringView name = "", Object* owner = nullptr, const Class* class_overload = nullptr) const;
		Object* create_placement_object(void* place, StringView name = "", Object* owner = nullptr,
		                                const Class* class_overload = nullptr) const;

		size_t sizeof_class() const;
		bool is_scriptable() const;
		Class& static_constructor(Object* (*new_static_constructor)(Class*, StringView, Object*) );
		Class& static_placement_constructor(Object* (*new_static_placement_constructor)(Class*, void*, StringView, Object*) );
		Object* singletone_instance() const;
		Class& post_initialize();

		using Struct::is_a;
		bool is_asset() const;
		bool is_class() const override;
		bool is_native() const;

		static Class* static_find(const StringView& name, bool required = false);
		static const Vector<Class*>& asset_classes();

		~Class();

		template<typename Type>
		bool is_a() const
		{
			return is_a(Type::static_class_instance());
		}

		template<typename ObjectClass>
		void setup_class()
		{
			if (m_size == 0)
			{
				m_size = sizeof(ObjectClass);
				flags(Flag::IsNative, true);

				m_script_factory = [](StringView name, Object* owner) -> Object* {
					return Object::new_instance<ObjectClass, true>(name, owner);
				};

				m_static_constructor = [](Class* self, StringView name, Object* owner) -> Object* {
					return Object::new_instance<ObjectClass, true>(name, owner);
				};

				m_static_placement_constructor = [](Class* self, void* place, StringView name, Object* owner) -> Object* {
					return Object::new_placement_instance<ObjectClass, true>(place, name, owner);
				};

				if constexpr (std::is_final_v<ObjectClass>)
				{
					flags(Flag::IsFinal, true);
				}

				if constexpr (std::is_abstract_v<ObjectClass>)
				{
					flags(Flag::IsAbstract, true);
				}

				if constexpr (is_singletone_v<ObjectClass>)
				{
					flags(Flag::IsSingletone, true);
				}

				if constexpr (!(std::is_abstract_v<ObjectClass> ||
				                (!std::is_constructible_v<ObjectClass> && !Engine::is_singletone_v<ObjectClass>) ))
				{
					flags(IsConstructible, true);
				}
			}
		}


		friend class ScriptClassRegistrar;
		friend class SingletoneBase;
	};

}// namespace Engine
