#pragma once

#include <Core/definitions.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/object.hpp>


namespace Engine
{
	class ENGINE_EXPORT SingletoneBase
	{
	protected:
		static void register_singletone(const Class* class_instance, Object* object);
		static void unlink_instance(const Class* class_instance);
		static Object* extract_object_from_class(const Class* class_instance);
	};

	struct EmptySingletoneParent {
	};

	template<typename Type, typename Parent = Object, bool with_destroy_controller = true>
	class Singletone : public Parent, public SingletoneBase
	{
	public:
		static constexpr inline bool singletone_based_on_object = std::is_base_of_v<Object, Type>;

		template<typename... Args>
		static Type* create_instance(Args&&... args)
		{
			if (!instance())
			{
				if constexpr (singletone_based_on_object)
				{
					register_singletone(Type::static_class_instance(), new Type(std::forward<Args>(args)...));
				}
				else
				{
					Type::m_instance = new Type(std::forward<Args>(args)...);

					if constexpr (with_destroy_controller)
					{
						PostDestroyController post_destroy([]() {
							if (Type::m_instance)
							{
								delete Type::m_instance;
								Type::m_instance = nullptr;
							}
						});
					}
				}
			}

			return instance();
		}

		template<typename... Args>
		static Type* create_placement_instance(void* place, Args&&... args)
		{
			if (!instance())
			{
				if constexpr (singletone_based_on_object)
				{
					register_singletone(Type::static_class_instance(), new (place) Type(std::forward<Args>(args)...));
				}
				else
				{
					Type::m_instance = new (place) Type(std::forward<Args>(args)...);
				}
			}

			return instance();
		}

		INLINE_DEBUGGABLE static Type* instance()
		{
			if constexpr (singletone_based_on_object)
			{
				Object* object = extract_object_from_class(Type::static_class_instance());

				if (object)
				{
					return object->instance_cast<Type>();
				}

				return nullptr;
			}
			else
			{
				return Type::m_instance;
			}
		}

		~Singletone()
		{
			if constexpr (!singletone_based_on_object)
			{
				Type::m_instance = nullptr;
			}
			else
			{
				unlink_instance(Type::static_class_instance());
			}
		}
	};
}// namespace Engine
