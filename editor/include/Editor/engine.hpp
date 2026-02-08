#pragma once
#include <Core/base_engine.hpp>
#include <Core/etl/flat_set.hpp>

namespace Engine
{
	class Actor;
	class Level;
	class World;

	class EditorEngine : public BaseEngine
	{
		trinex_class(EditorEngine, BaseEngine);

	private:
		FlatSet<Actor*> m_selected_actors;

	public:
		static EditorEngine* instance();

		const Vector<Actor*>& selected_actors() const;
		EditorEngine& select(Actor* actor);
		EditorEngine& unselect(Actor* actor);
		EditorEngine& unselect(Level* level);
		EditorEngine& unselect(World* world);
		EditorEngine& is_selected(Actor* actor, bool status);
		bool is_selected(Actor* actor) const;
	};
}// namespace Engine
