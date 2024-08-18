#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Systems/system.hpp>
#include <algorithm>
#include <cstring>


namespace Engine
{

	System::UpdateTask::UpdateTask(System* system, float dt) : m_system(system), m_dt(dt)
	{}

	int_t System::UpdateTask::execute()
	{
		m_system->update(m_dt);
		return sizeof(System::UpdateTask);
	}

	void System::on_create_fail()
	{
		throw EngineException(
				"Cannot create new system. Please, call Super::create(); in the overrided method 'create'");
	}

	void System::on_new_system(System* system)
	{
		system->create();
		if (!system->m_is_fully_created)
		{
			on_create_fail();
		}
	}

	System::System() : m_parent_system(nullptr)
	{
		flags(IsSerializable, false);
	}

	System& System::create()
	{
		const Class* _this	= This::static_class_instance();
		const Class* _class = class_instance();

		if (_class == nullptr || _this == nullptr || _this == _class)
		{
			throw EngineException("Each class based from Engine::System must be registered!");
		}

		rename(_class->base_name().c_str(), static_find_package("Engine::Systems", true));

		debug_log("System", "Created system '%s'", string_name().c_str());
		m_is_fully_created = true;
		return *this;
	}

	System& System::update(float dt)
	{
		for (System* system : m_subsystems)
		{
			system->update(dt);
		}
		return *this;
	}

	System& System::wait()
	{
		for (System* subsystem : m_subsystems)
		{
			subsystem->wait();
		}

		return *this;
	}

	System& System::register_subsystem(System* system)
	{
		if (system->parent_system() == this)
			return *this;

		if (system->m_parent_system)
		{
			system->m_parent_system->remove_subsystem(system);
		}

		m_subsystems.push_back(system);
		system->m_parent_system = this;
		system->owner(this);
		return *this;
	}

	System& System::remove_subsystem(System* system)
	{
		if (system->m_parent_system == this)
		{
			auto it	 = m_subsystems.begin();
			auto end = m_subsystems.end();

			while (it != end)
			{
				if (*it == system)
				{
					m_subsystems.erase(it);
					system->m_parent_system = nullptr;
					system->owner(nullptr);
					return *this;
				}
				++it;
			}

			system->owner(nullptr);
		}

		return *this;
	}

	System* System::parent_system() const
	{
		return m_parent_system;
	}


	static bool sort_systems_predicate(System* first, System* second)
	{
		Class* _first  = first->class_instance();
		Class* _second = second->depends_on();


		while (_first && _second)
		{
			if (_first == _second)
				return true;

			System* system = _second->singletone_instance()->instance_cast<System>();

			if (system)
			{
				_second = system->depends_on();
			}
			else
			{
				_second = nullptr;
			}
		}

		return false;
	}

	System& System::sort_subsystems()
	{
		std::sort(m_subsystems.begin(), m_subsystems.end(), sort_systems_predicate);
		std::for_each(m_subsystems.begin(), m_subsystems.end(), [](System* system) { system->sort_subsystems(); });
		return *this;
	}

	System* System::find_system_private_no_recurse(const char* _name, size_t len) const
	{
		for (System* system : m_subsystems)
		{
			const String& system_name = system->string_name();
			if (system_name.length() != len)
				continue;

			if (std::strncmp(_name, system_name.c_str(), len) == 0)
				return system;
		}

		return nullptr;
	}

	System* System::find_subsystem(const char* _name, size_t len)
	{
		const char* end_name	   = _name + len;
		const size_t separator_len = Constants::name_separator.length();
		const char* separator	   = Strings::strnstr(_name, len, Constants::name_separator.c_str(), separator_len);
		const System* system	   = this;


		while (separator && system)
		{
			size_t current_len = separator - _name;
			system			   = system->find_system_private_no_recurse(_name, current_len);
			_name			   = separator + separator_len;
			separator = Strings::strnstr(_name, end_name - _name, Constants::name_separator.c_str(), separator_len);
		}

		return system ? system->find_system_private_no_recurse(_name, end_name - _name) : nullptr;
	}

	System* System::find_subsystem(const char* name)
	{
		return find_subsystem(name, std::strlen(name));
	}

	System* System::find_subsystem(const String& name)
	{
		return find_subsystem(name.c_str(), name.length());
	}

	class Class* System::depends_on() const
	{
		return nullptr;
	}

	bool System::is_shutdowned() const
	{
		return !m_is_fully_created;
	}

	System& System::shutdown()
	{
		info_log("System", "Shutting down system %p", this);

		if (m_parent_system)
		{
			m_parent_system->remove_subsystem(this);
		}

		// Shutdown child systems

		Vector<System*> subsystems = std::move(m_subsystems);
		for (System* system : subsystems)
		{
			system->shutdown();
		}

		m_is_fully_created = false;
		return *this;
	}

	System* System::new_system(class Class* class_instance)
	{
		if (class_instance && class_instance->is_a(System::static_class_instance()))
		{
			System* system = class_instance->create_object()->instance_cast<System>();
			if (system && system->m_is_fully_created == false)
			{
				on_new_system(system);
			}
			return system;
		}

		return nullptr;
	}

	System* System::new_system(const String& name)
	{
		return new_system(Class::static_find(name));
	}

	const Vector<System*>& System::subsystems() const
	{
		return m_subsystems;
	}

	Identifier System::id() const
	{
		return reinterpret_cast<Identifier>(this);
	}

	System::~System()
	{
		if (m_is_fully_created)
		{
			error_log("System", "You must call shutdown method before destroy system! System address %p", this);
		}
	}

	static void on_system_destroy(Object* object)
	{
		if (System* system = object->instance_cast<System>())
		{
			if (!system->is_shutdowned())
			{
				system->shutdown();
			}
		}
	}

	implement_engine_class(System, Class::IsScriptable)
	{
		Class* self = static_class_instance();
		self->on_destroy.push(on_system_destroy);
	}
}// namespace Engine
