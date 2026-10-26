#pragma once
#include <Core/callback.hpp>
#include <Core/engine.hpp>
#include <Core/etl/flat_set.hpp>

namespace Trinex
{
	class Actor;
	class Level;
	class World;

	class EditorEngine : public Engine
	{
		trinex_class(EditorEngine, Engine);

	private:
		FlatSet<Actor*> m_selected_actors;

	public:
		CallBacks<void(Actor*)> on_actor_select;
		CallBacks<void(Actor*)> on_actor_unselect;

	public:
		static EditorEngine* instance();

		const Array<Actor*>& selected_actors() const;
		EditorEngine& select(Actor* actor);
		EditorEngine& unselect(Actor* actor);
		EditorEngine& unselect(Level* level);
		EditorEngine& unselect(World* world);
		EditorEngine& is_selected(Actor* actor, bool status);
		bool is_selected(Actor* actor) const;
	};
} // namespace Trinex
