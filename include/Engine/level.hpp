#pragma once
#include <Core/etl/deque.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>


namespace Engine
{
	class World;
	class Actor;

	class ENGINE_EXPORT Level : public Object
	{
		trinex_class(Level, Object);

	private:
		Vector<Actor*> m_actors;
		bool m_is_playing;


	private:
		void scriptable_start_play();
		void scriptable_update(float dt);
		void scriptable_stop_play();

	protected:
		bool register_child(Object* child, uint32_t& index) override;
		bool unregister_child(Object* child) override;

	public:
		template<typename NativeType>
		struct Scriptable : Super::Scriptable<NativeType> {
			Scriptable& start_play() override
			{
				static_cast<Level*>(this)->scriptable_start_play();
				return *this;
			}

			Scriptable& update(float dt) override
			{
				static_cast<Level*>(this)->scriptable_update(dt);
				return *this;
			}

			Scriptable& stop_play() override
			{
				static_cast<Level*>(this)->scriptable_stop_play();
				return *this;
			}
		};

	public:
		Level();
		~Level();

		virtual Level& spawned();
		virtual Level& start_play();
		virtual Level& update(float dt);
		virtual Level& stop_play();
		virtual Level& despawned();

		virtual World* world();

		inline bool is_playing() const { return m_is_playing; }
		inline const Vector<class Actor*>& actors() const { return m_actors; }
	};
}// namespace Engine
