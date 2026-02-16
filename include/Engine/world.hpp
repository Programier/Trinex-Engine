#pragma once
#include <Engine/level.hpp>

namespace Engine
{
	class Scene;

	class ENGINE_EXPORT World : public Level
	{
		trinex_class(World, Level);

	private:
		Vector<class Level*> m_levels;
		Scene* m_scene = nullptr;

	protected:
		bool register_child(Object* child, uint32_t& index) override;
		bool unregister_child(Object* child) override;

	public:
		World();
		~World();

		static World* current();

		World& start_play() override;
		World& update(float dt) override;
		World& stop_play() override;

		World* world() override;

		inline const Vector<class Level*>& levels() const { return m_levels; }
		inline Scene* scene() const { return m_scene; }
	};
}// namespace Engine
