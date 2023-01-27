#include <Core/logger.hpp>
#include <Graphics/animated_object.hpp>
#include <Graphics/animation.hpp>
#include <Graphics/animator.hpp>
#include <Graphics/bone.hpp>
#include <Graphics/drawable.hpp>
#include <cmath>


namespace Engine
{
    declare_instance_info_cpp(Animator);
    constructor_cpp(Animator, Animation* _animation)
    {
        animation(_animation);
    }

    Animator& Animator::animation(Animation* _animation)
    {
        _M_current_time = 0.0;
        _M_animation = _animation;
        return *this;
    }

    Animation* Animator::animation()
    {
        return _M_animation;
    }

    Animator& Animator::tick(float delta_time)
    {
        if (!_M_animation)
            return *this;

        current_time(std::fmod(_M_current_time + _M_animation->ticks_per_second * delta_time, _M_animation->duration));
        return *this;
    }

    Animator& Animator::current_time(float time)
    {
        _M_current_time = time;

        if (!_M_animation)
            return *this;

        for (auto& channel : _M_animation->channels())
        {
            channel->update(time);
            Bone* bone = channel->bone();
            if (bone)
                bone->model(channel->model() * bone->offset_matrix);
        }

        _M_animation->object()->update_animation_matrices();
        return *this;
    }

    float Animator::current_time() const
    {
        return _M_current_time;
    }


}// namespace Engine
