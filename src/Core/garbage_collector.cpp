#include <Core/base_engine.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/settings.hpp>

#define DISABLE_GC 1

namespace Engine
{
	enum GCStage : EnumerateType
	{
		MarkUnreachable   = 0,
		CollectingGarbage = 1,
		DestroyGarbage    = 2,

		FirstStage = MarkUnreachable,
		LastStage  = DestroyGarbage
	};

	static struct GCState {
		Index object_index = 0;
		GCStage stage      = GCStage::MarkUnreachable;
	} gc_state;

	CallBacks<void(Object*)> GarbageCollector::on_unreachable_check;
	CallBacks<void(Object*)> GarbageCollector::on_destroy;

	static FORCE_INLINE ByteAllocator& allocator()
	{
		static ByteAllocator object_allocator;
		return object_allocator;
	}

	void* Object::operator new(size_t size) noexcept
	{
		byte* memory = allocator().allocate(size);
		return memory;
	}

	ENGINE_EXPORT void* Object::operator new(size_t size, void* place) noexcept
	{
		return place;
	}

	void Object::operator delete(void* _memory, size_t size) noexcept
	{
		byte* memory = reinterpret_cast<byte*>(_memory);
		allocator().deallocate(memory, size);
	}

	static FORCE_INLINE uint32_t get_max_objects_per_tick()
	{
		return glm::max<uint32_t>(1, Settings::gc_max_object_per_tick);
	}


	void GarbageCollector::destroy_recursive(Object* object, bool destroy_owner_if_exist)
	{
		if (object == nullptr || object->flags(Object::IsAvailableForGC) == false)
			return;

		if (destroy_owner_if_exist)
		{
			if (Object* owner = object->owner())
			{
				destroy_recursive(owner, destroy_owner_if_exist);
			}
		}

		destroy(object);
	}

	ENGINE_EXPORT void GarbageCollector::destroy_internal(Object* object)
	{
		delete object;
	}

	void GarbageCollector::destroy(Object* object)
	{
		if (object == nullptr)
			return;

		if (engine_instance && !engine_instance->is_shuting_down())
		{
			if (!object->is_noname())
			{
				debug_log("Garbage Collector", "Destroy object '%s'", object->string_name().c_str());
			}
			else
			{
				debug_log("Garbage Collector", "Destroy noname object with type '%s'",
						  object->class_instance()->full_name().c_str());
			}
		}

		object->begin_destroy();
		on_destroy(object);
		object->class_instance()->destroy_object(object);
	}

	void GarbageCollector::update(float dt)
	{
		if (Object::all_objects().empty())
			return;

		if (gc_state.stage == GCStage::MarkUnreachable)
		{
			mark_unreachable(dt);
		}
		else if (gc_state.stage == GCStage::CollectingGarbage)
		{
			collect_garbage(dt);
		}
		else if (gc_state.stage == GCStage::DestroyGarbage)
		{
			destroy_garbage(dt);
		}
	}

	void GarbageCollector::submit_current_stage()
	{
		gc_state.object_index = 0;

		EnumerateType value = gc_state.stage + 1;
		if (value > LastStage)
		{
			value = FirstStage;
		}
		gc_state.stage = static_cast<GCStage>(value);
	}

	bool GarbageCollector::process_objects(void (*callback)(Object* object))
	{
		const auto& objects     = Object::all_objects();
		uint_t objects_per_tick = get_max_objects_per_tick();
		auto objects_count      = objects.size();

		while (objects_per_tick > 0 && gc_state.object_index < objects_count)
		{
			Object* object = objects[gc_state.object_index];

			if (object)
			{
				--objects_per_tick;
				callback(object);
			}

			++gc_state.object_index;
		}

		if (gc_state.object_index == objects_count)
		{
			submit_current_stage();
			return true;
		}
		return false;
	}

	void GarbageCollector::mark_unreachable(float dt)
	{
		static void (*callback)(Object* object) = [](Object* object) { object->flags(Object::Flag::IsUnreachable, true); };
		process_objects(callback);
	}

	static void find_unreachable(Object* object)
	{
		GarbageCollector::on_unreachable_check(object);

		auto& flags = object->flags;

		if (flags(Object::StandAlone) || object->references() > 0 || object->owner() != nullptr)
		{
			object->flags(Object::IsUnreachable, false);
		}

		if (!object->flags(Object::IsUnreachable))
		{
			Refl::Class* class_instance = object->class_instance();

			if (class_instance)
			{
				for (auto& prop : class_instance->properties())
				{
					if (!prop)
					{
						continue;
					}

					if (auto object_prop = Refl::Object::instance_cast<Refl::ObjectProperty>(prop))
					{
						Object* prop_object = object_prop->object(object);
						if (prop_object)
						{
							prop_object->flags(Object::IsUnreachable, false);
						}
					}
				}
			}
		}
	}


	void GarbageCollector::collect_garbage(float dt)
	{
		process_objects(find_unreachable);
	}

	void GarbageCollector::destroy_garbage(float dt)
	{
		static void (*callback)(Object* object) = [](Object* object) {
			if (object->flags(Object::Flag::IsUnreachable))
			{
				destroy(object);
			}
		};

		process_objects(callback);
	}

	void GarbageCollector::destroy_all_objects()
	{
		auto& objects = const_cast<ObjectArray&>(Object::all_objects());

		for (Object* object : objects)
		{
			destroy_recursive(object, true);
		}
	}
}// namespace Engine
