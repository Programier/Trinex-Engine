#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Event/listener_id.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{

	EventSystemListenerID::EventSystemListenerID() : m_type(EventType::Undefined), m_id(0), m_is_valid(false)
	{}

	EventSystemListenerID::EventSystemListenerID(EventType type, Identifier id) : m_type(type), m_id(id), m_is_valid(true)
	{}

	default_copy_constructors_cpp(EventSystemListenerID);

	bool EventSystemListenerID::is_valid() const
	{
		return m_is_valid;
	}
}// namespace Engine
