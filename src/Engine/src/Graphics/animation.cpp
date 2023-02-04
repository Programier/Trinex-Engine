#include <Core/assimp_helpers.hpp>
#include <Core/check.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/animation.hpp>
#include <Graphics/bone.hpp>
#include <assimp/scene.h>


namespace Engine
{
    declare_instance_info_cpp(Animation);

    constructor_cpp(Animation, AnimatedObject* object)
    {
        check(object);
        _M_object = object;
    }


    Animation::declare_instance_info_cpp(Channel);
    Animation::constructor_cpp(Channel)
    {}

#define load_keys(engine_keys, ai_keys, function)                                                                      \
    channel->engine_keys.clear();                                                                                      \
    channel->engine_keys.resize(ai_channel->mNum##ai_keys);                                                            \
    for (decltype(ai_channel->mNum##ai_keys) i = 0; i < ai_channel->mNum##ai_keys; i++)                                \
    {                                                                                                                  \
        channel->engine_keys[i].value = AssimpHelpers::function(&ai_channel->m##ai_keys[i].mValue);                    \
        channel->engine_keys[i].time = static_cast<float>(ai_channel->m##ai_keys[i].mTime);                            \
    }


    Animation& Animation::load(const aiAnimation* anim, const std::list<const aiNodeAnim*>& ai_channels, Bone* root)
    {
        name(anim->mName.data);
        duration = static_cast<float>(anim->mDuration);
        ticks_per_second = static_cast<float>(anim->mTicksPerSecond);

        for (auto& ai_channel : ai_channels)
        {
            _M_channels.push_back(Object::new_instance<Channel>());
            auto& channel = _M_channels.back();
            channel->name = ai_channel->mNodeName.data;
            channel->_M_bone = root ? root->find_bone_by_name(channel->name) : nullptr;

            load_keys(position_keys, PositionKeys, get_vector3);
            load_keys(rotation_keys, RotationKeys, get_quaternion);
            load_keys(scaling_keys, ScalingKeys, get_vector3);
        }

        return *this;
    }


    template<typename Type>
    static ArrayIndex get_index(float current_time, const std::vector<Animation::Channel::Key<Type>>& array)
    {
        // Binary search index
        ArrayIndex left = array.size();
        ArrayIndex right = 0;
        ArrayIndex index = Constants::index_none;

        while (right < left)
        {
            index = (left + right) / 2;
            float time = array[index].time;

            if (time < current_time)
                right = index + 1;
            else if (time > current_time)
                left = index == 0 ? index : index - 1;
            else
                return index;
        }

        index = (left + right) / 2;
        return (index == 0 || index == array.size()) ? Constants::index_none : index - 1;
    }

    Bone* Animation::Channel::bone() const
    {
        return _M_bone;
    }

    Animation::Channel& Animation::Channel::update(float current_time)
    {
        return interpolate_position(current_time).interpolate_scaling(current_time).interpolate_rotation(current_time);
    }

    ArrayIndex Animation::Channel::get_position_index(float current_time) const
    {
        return get_index(current_time, position_keys);
    }

    ArrayIndex Animation::Channel::get_rotation_index(float current_time) const
    {
        return get_index(current_time, rotation_keys);
    }

    ArrayIndex Animation::Channel::get_scaling_index(float current_time) const
    {
        return get_index(current_time, scaling_keys);
    }

#define get_scale_factor(last, next, current) ((current - last) / (next - last))

#define implement_interpolate(type, function, engine_method)                                                           \
    if (type##_keys.size() == 1)                                                                                       \
    {                                                                                                                  \
        engine_method(type##_keys[0].value, false);                                                                    \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    ArrayIndex index1 = get_##type##_index(current_time);                                                              \
    if (index1 == Constants::index_none)                                                                               \
        return *this;                                                                                                  \
                                                                                                                       \
    ArrayIndex index2 = index1 + 1;                                                                                    \
    float scale_factor = get_scale_factor(type##_keys[index1].time, type##_keys[index2].time, current_time);           \
    engine_method(glm::function(type##_keys[index1].value, type##_keys[index2].value, scale_factor), false);

    Animation::Channel& Animation::Channel::interpolate_position(float current_time)
    {
        implement_interpolate(position, mix, move);
        return *this;
    }

    Animation::Channel& Animation::Channel::interpolate_rotation(float current_time)
    {
        implement_interpolate(rotation, slerp, rotate);
        return *this;
    }

    Animation::Channel& Animation::Channel::interpolate_scaling(float current_time)
    {
        implement_interpolate(scaling, mix, scale);
        return *this;
    }

    const Animation::ChannelsArray& Animation::channels() const
    {
        return _M_channels;
    }

    Animation::ChannelsArray& Animation::channels()
    {
        return _M_channels;
    }

    AnimatedObject* Animation::object() const
    {
        return _M_object;
    }
}// namespace Engine
