#pragma once
#include <Core/callback.hpp>
#include <Core/object.hpp>
#include <Core/reflection/struct.hpp>

namespace Engine
{
	class ScriptClassRegistrar;
	class SingletoneBase;

	namespace Refl
	{
		class ENGINE_EXPORT Class : public Struct
		{
			trinex_reflect_type(Class, Struct);

		private:
			mutable Engine::Object* m_singletone_object;

		private:
			void script_object_constructor(void* object, StringView name = "", Engine::Object* owner = nullptr);
			inline void script_object_constructor_default(void* object) { script_object_constructor(object); }

		protected:
			static bool is_script_class(Class* self);
			using ObjectFactory = Engine::Object*(StringView name, Engine::Object* owner);

			virtual Engine::Object* object_constructor(StringView name = "", Engine::Object* owner = nullptr,
			                                           bool scriptable = false);
			virtual Engine::Object* object_placement_constructor(void* mem, StringView name = "", Engine::Object* owner = nullptr,
			                                                     bool scriptable = false);
			virtual ObjectFactory* script_object_factory() const;

			Class& register_scriptable_instance() override;
			Class& initialize() override;

		public:
			Class(Class* parent = nullptr, BitMask flags = 0);

			Class* parent() const;

			void* create_struct() override;
			Class& destroy_struct(void* obj) override;

			Engine::Object* create_object(StringView name = "", Engine::Object* owner = nullptr);
			Engine::Object* create_placement_object(void* place, StringView name = "", Engine::Object* owner = nullptr);
			virtual Class& destroy_object(Engine::Object* object);
			Engine::Object* singletone_instance() const;

			using Struct::is_a;
			const ScriptTypeInfo& find_valid_script_type_info() const;
			static const Vector<Class*>& asset_classes();
			static void register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast);

			template<typename Type>
			bool is_a() const
			    requires(std::is_base_of_v<Engine::Object, Type>)
			{
				return is_a(Type::static_reflection());
			}

			friend class Engine::ScriptClassRegistrar;
			friend class Engine::SingletoneBase;
		};

		template<typename T>
		class NativeClass : public Class
		{
			using ScriptableType = T::template Scriptable<T>;

			ObjectFactory* script_object_factory() const override
			{
				if constexpr (!is_singletone_v<T>)
				{
					if constexpr (std::is_final_v<T>)
					{
						return [](StringView name, Engine::Object* owner) -> Engine::Object* {
							return Engine::Object::new_instance<T, true>(name, owner);
						};
					}
					else
					{
						return [](StringView name, Engine::Object* owner) -> Engine::Object* {
							return Engine::Object::new_instance<ScriptableType, true>(name, owner);
						};
					}
				}

				return nullptr;
			}

			Engine::Object* object_constructor(StringView name, Engine::Object* owner, bool scriptable = false) override
			{
				if constexpr (!std::is_final_v<T> && !is_singletone_v<T>)
				{
					if (scriptable)
					{
						return Engine::Object::new_instance<ScriptableType, true>(name, owner);
					}
				}
				return Engine::Object::new_instance<T, true>(name, owner);
			}

			Engine::Object* object_placement_constructor(void* mem, StringView name, Engine::Object* owner,
			                                             bool scriptable = false) override
			{
				if constexpr (!std::is_final_v<T> && !is_singletone_v<T>)
				{
					if (scriptable)
					{
						return Engine::Object::new_placement_instance<ScriptableType, true>(mem, name, owner);
					}
				}
				return Engine::Object::new_placement_instance<T, true>(mem, name, owner);
			}

		public:
			NativeClass(Class* parent = nullptr, BitMask flags = 0) : Class(parent, flags) {}

			static Class* create(StringView decl, BitMask flags = 0)
			{
				Class* parent = nullptr;

				if constexpr (!std::is_same_v<T, Engine::Object>)
				{
					parent = T::Super::static_reflection();
				}

				if (NativeClass* self = Object::new_instance<NativeClass<T>>(decl, parent, flags | native_type_flags<T>()))
				{
					return self;
				}

				return nullptr;
			}

			NativeClass& initialize() override
			{
				Class::initialize();
				T::static_initialize_class();
				return *this;
			}

			size_t size() const override { return sizeof(T); }
		};
	}// namespace Refl
}// namespace Engine
