#pragma once
#include <Core/callback.hpp>
#include <Core/deprecated/object.hpp>
#include <Core/reflection/deprecated/struct.hpp>

namespace Trinex
{
	namespace ScriptBinding
	{
		class Class;
	}
	class SingletoneBase;

	namespace Refl::Deprecated
	{
		class trinex_api Class : public Struct
		{
			trinex_reflect_type(Class, Struct);

		private:
			mutable Trinex::DeprecatedObject* m_singletone_object;

		private:
			void script_object_constructor(void* object, StringView name = "", Trinex::DeprecatedObject* owner = nullptr);
			inline void script_object_constructor_default(void* object) { script_object_constructor(object); }

		protected:
			static bool is_script_class(Class* self);
			using ObjectFactory = Trinex::DeprecatedObject*(StringView name, Trinex::DeprecatedObject* owner);

			virtual Trinex::DeprecatedObject* object_constructor(StringView name = "", Trinex::DeprecatedObject* owner = nullptr,
			                                                     bool scriptable = false);
			virtual Trinex::DeprecatedObject* object_placement_constructor(void* mem, StringView name = "",
			                                                               Trinex::DeprecatedObject* owner = nullptr,
			                                                               bool scriptable                 = false);
			virtual ObjectFactory* script_object_factory() const;

			Class& register_scriptable_instance() override;
			Class& initialize() override;

		public:
			Class(Class* parent = nullptr, Struct::Flags flags = 0);

			Class* parent() const;

			void* create_struct() override;
			Class& destroy_struct(void* obj) override;

			Trinex::DeprecatedObject* create_object(StringView name = "", Trinex::DeprecatedObject* owner = nullptr);
			Trinex::DeprecatedObject* create_placement_object(void* place, StringView name = "",
			                                                  Trinex::DeprecatedObject* owner = nullptr);
			virtual Class& destroy_object(Trinex::DeprecatedObject* object);
			Trinex::DeprecatedObject* singletone_instance() const;

			using Struct::is_a;
			const ScriptTypeInfo& find_valid_script_type_info() const;
			static const Array<Class*>& asset_classes();
			static void register_layout(ScriptBinding::Class& r, ClassInfo* info, DownCast downcast);

			template<typename Type>
			bool is_a() const
			    requires(std::is_base_of_v<Trinex::DeprecatedObject, Type>)
			{
				return is_a(Type::static_reflection());
			}

			friend class Trinex::ScriptBinding::Class;
			friend class Trinex::SingletoneBase;
		};

		template<typename T>
		class NativeClass : public Class
		{
			using ScriptableType = T::template Scriptable<T>;

			ObjectFactory* script_object_factory() const override
			{
				if constexpr (std::is_final_v<T>)
				{
					return [](StringView name, Trinex::DeprecatedObject* owner) -> Trinex::DeprecatedObject* {
						return Trinex::DeprecatedObject::new_instance<T, true>(name, owner);
					};
				}
				else
				{
					return [](StringView name, Trinex::DeprecatedObject* owner) -> Trinex::DeprecatedObject* {
						return Trinex::DeprecatedObject::new_instance<ScriptableType, true>(name, owner);
					};
				}

				return nullptr;
			}

			Trinex::DeprecatedObject* object_constructor(StringView name, Trinex::DeprecatedObject* owner,
			                                             bool scriptable = false) override
			{
				if constexpr (!std::is_final_v<T>)
				{
					if (scriptable)
					{
						return Trinex::DeprecatedObject::new_instance<ScriptableType, true>(name, owner);
					}
				}
				return Trinex::DeprecatedObject::new_instance<T, true>(name, owner);
			}

			Trinex::DeprecatedObject* object_placement_constructor(void* mem, StringView name, Trinex::DeprecatedObject* owner,
			                                                       bool scriptable = false) override
			{
				if constexpr (!std::is_final_v<T>)
				{
					if (scriptable)
					{
						return Trinex::DeprecatedObject::new_placement_instance<ScriptableType, true>(mem, name, owner);
					}
				}
				return Trinex::DeprecatedObject::new_placement_instance<T, true>(mem, name, owner);
			}

		public:
			NativeClass(Class* parent = nullptr, Struct::Flags flags = 0) : Class(parent, flags) {}

			static Class* create(StringView decl, Struct::Flags flags = 0)
			{
				Class* parent = nullptr;

				if constexpr (!std::is_same_v<T, Trinex::DeprecatedObject>)
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
	} // namespace Refl::Deprecated
} // namespace Trinex
