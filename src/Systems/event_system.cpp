#include <Core/base_engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/keyboard.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_viewport.hpp>
#include <Platform/platform.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Systems/game_controller_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Systems/touchscreen_system.hpp>


namespace Engine
{
	EventSystem::EventSystem()
	{
		std::memset(m_listeners, 0, sizeof(m_listeners));
	}

	Identifier EventSystem::add_listener(EventType type, const Listener& listener)
	{
		size_t index       = static_cast<size_t>(type);
		ListenerNode* node = new ListenerNode();
		node->type         = type;
		node->listener     = listener;

		ListenerNode*& current = m_listeners[index];

		if (current)
		{
			current->prev = node;
			node->next    = current;
		}

		current = node;
		return node->id();
	}

	EventSystem& EventSystem::remove_listener(Identifier id)
	{
		logic_thread()->call([node = reinterpret_cast<ListenerNode*>(id)]() {
			if (EventSystem* system = EventSystem::instance())
			{
				if (node->prev)
				{
					node->prev->next = node->next;
				}

				if (node->next)
				{
					node->next->prev = node->prev;
				}

				if (system->m_listeners[node->index()] == node)
				{
					system->m_listeners[node->index()] = node->next;
				}

				delete node;
			}
		});

		return *this;
	}

	EventSystem& EventSystem::create()
	{
		Super::create();

		System::system_of<EngineSystem>()->register_subsystem(this);

		// Register subsystems
		system_of<KeyboardSystem>();
		system_of<MouseSystem>();
		system_of<TouchScreenSystem>();
		system_of<GameControllerSystem>();

		process_event_method(ProcessEventMethod::PoolEvents);

		return *this;
	}

	EventSystem& EventSystem::update(float dt)
	{
		Super::update(dt);
		(this->*m_process_events)();
		return *this;
	}

	EventSystem& EventSystem::execute_listeners(ListenerNode* node, const Event& event)
	{
		while (node)
		{
			node->listener(event);
			node = node->next;
		}
		return *this;
	}

	EventSystem& EventSystem::push_event(const Event& event)
	{
		size_t index  = static_cast<size_t>(event.type);
		size_t index2 = static_cast<size_t>(EventType::Undefined);
		execute_listeners(m_listeners[index], event);
		execute_listeners(m_listeners[index2], event);
		return *this;
	}

	EventSystem& EventSystem::shutdown()
	{
		Super::shutdown();

		for (auto& listener : m_listeners)
		{
			while (listener)
			{
				ListenerNode* next = listener->next;
				delete listener;
				listener = next;
			}
		}

		return *this;
	}

	Name EventSystem::event_name(EventType type)
	{
		static Refl::Enum* event_type_enum = Refl::Enum::static_find("Engine::EventType", Refl::FindFlags::IsRequired);
		auto entry                         = event_type_enum->entry(static_cast<EnumerateType>(type));

		if (entry)
		{
			return entry->name;
		}

		return Name::undefined;
	}

	static void push_event_internal(const Event& event, void* self)
	{
		reinterpret_cast<EventSystem*>(self)->push_event(event);
	}

	EventSystem& EventSystem::wait_events()
	{
		Platform::EventSystem::wait_for_events(push_event_internal, this);
		return *this;
	}

	EventSystem& EventSystem::pool_events()
	{
		Platform::EventSystem::pool_events(push_event_internal, this);
		return *this;
	}

	EventSystem& EventSystem::process_event_method(ProcessEventMethod method)
	{
		if (method == ProcessEventMethod::PoolEvents)
		{
			m_process_events = &EventSystem::pool_events;
		}
		else
		{
			m_process_events = &EventSystem::wait_events;
		}
		return *this;
	}

	static Identifier script_add_listener(EventSystem* system, EventType type, asIScriptFunction* script_func)
	{
		Pointer<Object> object;

		if (auto obj_type = script_func->GetDelegateObjectType())
		{
			if (obj_type->DerivesFrom(Object::static_reflection()->script_type_info.info()))
			{
				object = reinterpret_cast<Object*>(script_func->GetDelegateObject());
			}
		}

		auto listener = [function = ScriptFunction(script_func), object](const Event& event) {
			ScriptContext::execute(function, nullptr, &event);
		};

		script_func->Release();
		return system->add_listener(type, listener);
	}

	trinex_implement_engine_class(EventSystem, Refl::Class::IsScriptable)
	{
		ReflectionInitializeController().require("Engine::Event");

		auto reg = ScriptClassRegistrar::existing_class(static_reflection());

		reg.funcdef("void Listener(const Engine::Event&)");
		reg.static_function("EventSystem@ instance()", instance);
		reg.method("uint64 add_listener(EventType type, EventSystem::Listener@ listener) final", script_add_listener);
		reg.method("void remove_listener(uint64 listener)", &EventSystem::remove_listener);
	}
}// namespace Engine
