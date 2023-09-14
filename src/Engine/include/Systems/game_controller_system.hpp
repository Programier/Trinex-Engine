#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/system.hpp>
#include <Event/event.hpp>

namespace Engine
{
    class GameController;

    class GameControllerSystem : public Singletone<GameControllerSystem, System>
    {
        declare_class(GameControllerSystem, System);


    private:
        static GameControllerSystem* _M_instance;

        Map<Identifier, GameController*> _M_controllers;
        Vector<Identifier> _M_callbacks_id;

        void on_controller_added(const Event& event);
        void on_controller_removed(const Event& event);
        void on_axis_motion(const Event& event);


        GameControllerSystem();

    public:
        virtual GameControllerSystem& create() override;
        virtual void wait() override;
        virtual GameControllerSystem& update(float dt) override;
        virtual GameControllerSystem& shutdown() override;
        GameController* controller(Identifier id) const;

        friend class Singletone<GameControllerSystem, System>;
        friend class Object;
    };
}// namespace Engine
