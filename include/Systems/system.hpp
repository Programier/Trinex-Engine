#pragma once
#include <Core/object.hpp>

namespace Engine
{
	class ENGINE_EXPORT System : public Object
	{
		declare_class(System, Object);

	private:
		bool m_is_initialized = false;

		static void on_create_fail();
		static void on_new_system(System* system);
		System* find_system_private_no_recurse(const char* name, size_t len) const;

	protected:
		Vector<System*> m_subsystems;
		System* m_parent_system;

	public:
		System();
		virtual System& create();
		virtual System& wait();
		virtual System& update(float dt);
		virtual System& shutdown();
		System& begin_destroy() override;
		static System* new_system(const String& name);
		static System* new_system(class Refl::Class* class_instance);

		const Vector<System*>& subsystems() const;
		System& register_subsystem(System* system);
		System& remove_subsystem(System* system);
		System* parent_system() const;
		System& sort_subsystems();
		System* find_subsystem(const char* name, size_t len);
		System* find_subsystem(const char* name);
		System* find_subsystem(const String& name);
		virtual class Refl::Class* depends_on() const;
		bool is_shutdowned() const;

		template<typename SystemType>
		static SystemType* new_system()
		{
			return reinterpret_cast<SystemType*>(new_system(SystemType::static_class_instance()));
		}
		~System();
		Identifier id() const;
	};
}// namespace Engine
