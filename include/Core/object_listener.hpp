#pragma once
#include <Core/etl/registry.hpp>

namespace Trinex
{
	class Object;

	class trinex_api ObjectCreateListener : public Registry<ObjectCreateListener>
	{
		trinex_registry(ObjectCreateListener);

	public:
		virtual ObjectCreateListener& on_object_create(Object* object) = 0;

		using Registry::for_each_invoke;
		static inline void for_each_invoke(Object* object)
		{
			Registry::for_each_invoke(&ObjectCreateListener::on_object_create, object);
		}
	};

	class trinex_api ObjectDestroyListener : public Registry<ObjectDestroyListener>
	{
		trinex_registry(ObjectDestroyListener);

	public:
		virtual ObjectDestroyListener& on_object_destroy(Object* object) = 0;

		using Registry::for_each_invoke;
		static inline void for_each_invoke(Object* object)
		{
			Registry::for_each_invoke(&ObjectDestroyListener::on_object_destroy, object);
		}
	};
} // namespace Trinex
