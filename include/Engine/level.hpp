#pragma once
#include <Core/etl/array.hpp>
#include <Core/deprecated/object.hpp>
#include <Core/pointer.hpp>

namespace Trinex
{
	class World;
	class Level;
	class Actor;

	class trinex_api LevelInstance : public DeprecatedObject
	{
		trinex_class(LevelInstance, DeprecatedObject);

	private:
		Pointer<Level> m_level;
		Array<Actor*> m_actors;
		bool m_is_playing;
		bool m_is_visible;

	private:
		void scriptable_start_play();
		void scriptable_update(float dt);
		void scriptable_stop_play();

	protected:
		DeprecatedObject* register_child(DeprecatedObject* child, u32& index) override;
		bool unregister_child(DeprecatedObject* child) override;

	public:
		template<typename NativeType>
		struct Scriptable : Super::Scriptable<NativeType>
		{
			Scriptable& start_play() override
			{
				static_cast<LevelInstance*>(this)->scriptable_start_play();
				return *this;
			}

			Scriptable& update(float dt) override
			{
				static_cast<LevelInstance*>(this)->scriptable_update(dt);
				return *this;
			}

			Scriptable& stop_play() override
			{
				static_cast<LevelInstance*>(this)->scriptable_stop_play();
				return *this;
			}
		};

	public:
		LevelInstance();
		~LevelInstance();

		virtual LevelInstance& spawned();
		virtual LevelInstance& start_play();
		virtual LevelInstance& update(float dt);
		virtual LevelInstance& stop_play();
		virtual LevelInstance& despawned();
		virtual World* world();

		bool serialize(Archive& archive) override;

		inline Level* level() const { return m_level; }
		inline bool is_visible() const { return m_is_visible; }
		inline LevelInstance& is_visible(bool visible) { trinex_this_return(m_is_visible = visible); }
		inline bool is_playing() const { return m_is_playing; }
		inline const Array<class Actor*>& actors() const { return m_actors; }
	};

	class trinex_api Level : public DeprecatedObject
	{
		trinex_class(Level, DeprecatedObject);

	private:
		Refl::Deprecated::Class* m_class;
		Buffer m_state;

	public:
		Level();

		LevelInstance* create_instance(StringView name = "", DeprecatedObject* owner = nullptr);
		bool update(LevelInstance* instance);
		bool serialize(Archive& ar) override;
	};
} // namespace Trinex
