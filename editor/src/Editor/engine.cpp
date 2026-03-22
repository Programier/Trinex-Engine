#include <Core/reflection/class.hpp>
#include <Editor/engine.hpp>

namespace Trinex
{
	trinex_implement_class(Trinex::EditorEngine, 0) {}

	EditorEngine* EditorEngine::instance()
	{
		return instance_cast<EditorEngine>(engine_instance);
	}

	const Vector<Actor*>& EditorEngine::selected_actors() const
	{
		return m_selected_actors.as_vector();
	}

	EditorEngine& EditorEngine::select(Actor* actor)
	{
		m_selected_actors.insert(actor);
		on_actor_select(actor);
		return *this;
	}

	EditorEngine& EditorEngine::unselect(Actor* actor)
	{
		on_actor_unselect(actor);
		m_selected_actors.erase(actor);
		return *this;
	}

	EditorEngine& EditorEngine::unselect(Level* level)
	{
		return *this;
	}

	EditorEngine& EditorEngine::unselect(World* world)
	{
		for (Actor* actor : m_selected_actors)
		{
			on_actor_unselect(actor);
		}
		m_selected_actors.clear();
		return *this;
	}

	EditorEngine& EditorEngine::is_selected(Actor* actor, bool status)
	{
		return status ? select(actor) : unselect(actor);
	}

	bool EditorEngine::is_selected(Actor* actor) const
	{
		return m_selected_actors.contains(actor);
	}
}// namespace Trinex
