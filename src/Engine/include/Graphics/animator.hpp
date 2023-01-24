#pragma once
#include <Core/export.hpp>
#include <Core/object.hpp>

namespace Engine
{
    class Animation;
    ENGINE_EXPORT class Animator : public Object
    {
    private:
        Animation* _M_animation = nullptr;

        float _M_current_time;

        declare_instance_info_hpp(Animator);
    public:
        delete_copy_constructors(Animator);
        constructor_hpp(Animator, Animation * animation = nullptr);
        Animator& animation(Animation * animation);
        Animation* animation();
        const Animation* animation() const;
        Animator& tick(float delta_time);
        float current_time() const;
        Animator& current_time(float time);
    };
}// namespace Engine
