#pragma once
#include <Core/callback.hpp>
#include <Core/object.hpp>
#include <Core/reflection/struct.hpp>

namespace Trinex
{
	class ScriptClassRegistrar;
	class SingletoneBase;

	namespace Refl
	{
		class ENGINE_EXPORT Class : public Struct
		{
			trinex_reflect_type(Class, Struct);

		private:
			mutable Trinex::Object* m_singletone_object;

		private:
			void script_object_constructor(void* object, StringView name = "", Trinex::Object* owner = nullptr);
			inline void script_object_constructor_default(void* object) { script_object_constructor(object); }

		protected:
			static bool is_script_class(Class* self);
			using ObjectFactory = Trinex::Object*(StringView name, Trinex::Object* owner);

			virtual Trinex::Object* object_constructor(StringView name = "", Trinex::Object* owner = nullptr,
			                                           bool scriptable = false);
			virtual Trinex::Object* object_placement_constructor(void* mem, StringView name = "", Trinex::Object* owner = nullptr,
			                                                     bool scriptable = false);
			virtual ObjectFactory* script_object_factory() const;

			Class& register_scriptable_instance() override;
			Class& initialize() override;

		public:
			Class(Class* parent = nullptr, BitMask flags = 0);

			Class* parent() const;

			void* create_struct() override;
			Class& destroy_struct(void* obj) override;

			Trinex::Object* create_object(StringView name = "", Trinex::Object* owner = nullptr);
			Trinex::Object* create_placement_object(void* place, StringView name = "", Trinex::Object* owner = nullptr);
			virtual Class& destroy_object(Trinex::Object* object);
			Trinex::Object* singletone_instance() const;

			using Struct::is_a;
			const ScriptTypeInfo& find_valid_script_type_info() const;
			static const Vector<Class*>& asset_classes();
			static void register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast);

			template<typename Type>
			bool is_a() const
			    requires(std::is_base_of_v<Trinex::Object, Type>)
			{
				return is_a(Type::static_reflection());
			}

			friend class Trinex::ScriptClassRegistrar;
			friend class Trinex::SingletoneBase;
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
						return [](StringView name, Trinex::Object* owner) -> Trinex::Object* {
							return Trinex::Object::new_instance<T, true>(name, owner);
						};
					}
					else
					{
						return [](StringView name, Trinex::Object* owner) -> Trinex::Object* {
							return Trinex::Object::new_instance<ScriptableType, true>(name, owner);
						};
					}
				}

				return nullptr;
			}

			Trinex::Object* object_constructor(StringView name, Trinex::Object* owner, bool scriptable = false) override
			{
				if constexpr (!std::is_final_v<T> && !is_singletone_v<T>)
				{
					if (scriptable)
					{
						return Trinex::Object::new_instance<ScriptableType, true>(name, owner);
					}
				}
				return Trinex::Object::new_instance<T, true>(name, owner);
			}

			Trinex::Object* object_placement_constructor(void* mem, StringView name, Trinex::Object* owner,
			                                             bool scriptable = false) override
			{
				if constexpr (!std::is_final_v<T> && !is_singletone_v<T>)
				{
					if (scriptable)
					{
						return Trinex::Object::new_placement_instance<ScriptableType, true>(mem, name, owner);
					}
				}
				return Trinex::Object::new_placement_instance<T, true>(mem, name, owner);
			}

		public:
			NativeClass(Class* parent = nullptr, BitMask flags = 0) : Class(parent, flags) {}

			static Class* create(StringView decl, BitMask flags = 0)
			{
				Class* parent = nullptr;

				if constexpr (!std::is_same_v<T, Trinex::Object>)
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

			usize size() const override { return sizeof(T); }
		};
	}// namespace Refl
}// namespace Trinex
