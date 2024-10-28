#pragma once
#include <Core/callback.hpp>
#include <Core/flags.hpp>
#include <Core/object.hpp>
#include <Core/reflection/struct.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine
{
	class ScriptClassRegistrar;
	class SingletoneBase;

	namespace Refl
	{
		class ENGINE_EXPORT Class : public Struct
		{
			declare_reflect_type(Class, Struct);

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

			Flags<Flag> flags;

		private:
			mutable Engine::Object* m_singletone_object;

			Engine::Object* (*m_script_factory)(StringView, Engine::Object*)                              = nullptr;
			Engine::Object* (*m_script_placement_constructor)(Class*, void*, StringView, Engine::Object*) = nullptr;

			void (*m_destroy_func)(Engine::Object*)                                                       = nullptr;
			Engine::Object* (*m_static_constructor)(Class*, StringView, Engine::Object*)                  = nullptr;
			Engine::Object* (*m_static_placement_constructor)(Class*, void*, StringView, Engine::Object*) = nullptr;

			size_t m_size;

			void on_create_call(Engine::Object* object) const;
			void bind_class_to_script_engine();
			void register_scriptable_class();
			static Class* create_internal(StringView decl, Class* parent = nullptr, BitMask flags = 0, StringView type_name = "");

		public:
			ScriptTypeInfo script_type_info;
			CallBacks<void(Engine::Object*)> on_create;
			CallBacks<void(Engine::Object*)> on_destroy;
			CallBacks<void(Class*)> on_class_destroy;
			Function<void(ScriptClassRegistrar*, Class*)> script_registration_callback;

			Class(Class* parent = nullptr, BitMask flags = 0, StringView type_name = "");

			Class* parent() const;

			void (*destroy_func() const)(Engine::Object*);
			Class& destroy_func(void (*)(Engine::Object*));

			void* create_struct() const override;
			const Struct& destroy_struct(void* obj) const override;
			Engine::Object* create_object(StringView name = "", Engine::Object* owner = nullptr,
										  const Class* class_overload = nullptr) const;
			Engine::Object* create_placement_object(void* place, StringView name = "", Engine::Object* owner = nullptr,
													const Class* class_overload = nullptr) const;

			size_t sizeof_class() const;
			bool is_scriptable() const;
			Class& static_constructor(Engine::Object* (*new_static_constructor)(Class*, StringView, Engine::Object*) );
			Engine::Object* singletone_instance() const;
			Class& post_initialize();

			using Struct::is_a;
			bool is_asset() const;
			bool is_native() const;

			const ScriptTypeInfo& find_valid_script_type_info() const;
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

					if constexpr (!is_singletone_v<ObjectClass>)
					{
						if (flags(Flag::IsScriptable))
						{
							if constexpr (std::is_final_v<ObjectClass>)
							{
								m_script_factory = [](StringView name, Engine::Object* owner) -> Engine::Object* {
									return Engine::Object::new_instance<ObjectClass, true>(name, owner);
								};

								m_script_placement_constructor = [](Class* self, void* place, StringView name,
																	Engine::Object* owner) -> Engine::Object* {
									return Engine::Object::new_placement_instance<ObjectClass, true>(place, name, owner);
								};
							}
							else
							{
								using ScriptableType = typename ObjectClass::template Scriptable<ObjectClass>;

								m_script_factory = [](StringView name, Engine::Object* owner) -> Engine::Object* {
									return Engine::Object::new_instance<ScriptableType, true>(name, owner);
								};

								m_script_placement_constructor = [](Class* self, void* place, StringView name,
																	Engine::Object* owner) -> Engine::Object* {
									return Engine::Object::new_placement_instance<ScriptableType, true>(place, name, owner);
								};
							}
						}
					}

					m_static_constructor = [](Class* self, StringView name, Engine::Object* owner) -> Engine::Object* {
						return Engine::Object::new_instance<ObjectClass, true>(name, owner);
					};

					m_static_placement_constructor = [](Class* self, void* place, StringView name,
														Engine::Object* owner) -> Engine::Object* {
						return Engine::Object::new_placement_instance<ObjectClass, true>(place, name, owner);
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

			template<typename T>
			static Class* create(StringView decl, BitMask flags = 0)
			{
				Class* parent = nullptr;

				if constexpr (!std::is_same_v<T, Engine::Object>)
				{
					parent = T::Super::static_class_instance();
				}

				if (Class* self = create_internal(decl, parent, flags, type_info<T>::name()))
				{
					self->setup_class<T>();
					return self;
				}
				return nullptr;
			}

			friend class Engine::ScriptClassRegistrar;
			friend class Engine::SingletoneBase;
		};
	}// namespace Refl
}// namespace Engine
