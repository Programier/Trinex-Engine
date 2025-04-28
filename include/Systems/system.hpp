#pragma once
#include <Core/object.hpp>

namespace Engine
{
	class ENGINE_EXPORT System : public Object
	{
		trinex_declare_class(System, Object);

	private:
		bool m_is_initialized = false;

		static void on_create_fail();
		static void on_new_system(System* system);
		System* find_system_private_no_recurse(const char* name, size_t len) const;

	protected:
		Vector<System*> m_subsystems;
		System* m_parent_system;

		virtual System& create();

	public:
		System();

		virtual System& wait();
		virtual System& update(float dt);
		virtual System& shutdown();
		System& on_destroy() override;
		static System* system_of(const String& name, Object* owner = nullptr);
		static System* system_of(class Refl::Class* class_instance, Object* owner = nullptr);

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
		static SystemType* system_of(Object* owner = nullptr)
		{
			return reinterpret_cast<SystemType*>(system_of(SystemType::static_class_instance(), owner));
		}
		~System();
		Identifier id() const;
	};
}// namespace Engine
