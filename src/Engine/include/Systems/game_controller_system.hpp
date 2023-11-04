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
        Map<Identifier, GameController*> _M_controllers;
        Vector<Identifier> _M_callbacks_id;

        void on_controller_added(const Event& event);
        void on_controller_removed(const Event& event);
        void on_axis_motion(const Event& event);


        GameControllerSystem();

    public:
        virtual GameControllerSystem& create() override;
        virtual GameControllerSystem& wait() override;
        virtual GameControllerSystem& update(float dt) override;
        virtual GameControllerSystem& shutdown() override;
        GameController* controller(Identifier id) const;

        friend class Object;
    };
}// namespace Engine
