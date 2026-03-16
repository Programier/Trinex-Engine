#pragma once
#include <Engine/level.hpp>

namespace Trinex
{
	class Scene;

	class ENGINE_EXPORT World : public LevelInstance
	{
		trinex_class(World, LevelInstance);

	private:
		Vector<LevelInstance*> m_levels;
		LevelInstance* m_active;
		Scene* m_scene = nullptr;

	protected:
		Object* register_child(Object* child, u32& index) override;
		bool unregister_child(Object* child) override;

	public:
		World();
		~World();

		static World* current();

		World& start_play() override;
		World& update(float dt) override;
		World& stop_play() override;

		World* world() override;
		World& active_level(LevelInstance* instance);

		inline LevelInstance* active_level() const { return m_active; }
		inline const Vector<LevelInstance*>& levels() const { return m_levels; }
		inline Scene* scene() const { return m_scene; }
	};
}// namespace Trinex
